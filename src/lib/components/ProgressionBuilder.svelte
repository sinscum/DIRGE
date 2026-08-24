<script lang="ts">
	import { untrack } from 'svelte';
	import { enhance } from '$app/forms';
	import Staff from '$lib/components/Staff.svelte';
	import type { Song } from '$lib/schemas';
	import {
		KEYS,
		MODES,
		TEMPO_BANDS,
		chordFromSymbol,
		chromaticMediants,
		diatonicTriads,
		findMode,
		noteName,
		parseNote,
		prettyNote,
		qualityLabel,
		renumber,
		totalBars,
		tritoneAbove,
		type Chord
	} from '$lib/music';
	type SaveResult = {
		saved?: boolean;
		message?: string;
		issues?: { field: string; message: string }[];
	} | null;

	// Keyed on song id by the page, so the starting values are read once on purpose.
	let {
		song,
		file,
		form = null
	}: { song: Song; file: string; form?: SaveResult } = $props();

	const DURATIONS = [1, 2, 4, 8, 16];

	let key = $state(untrack(() => song.key));
	let mode = $state(untrack(() => song.mode));
	let bpm = $state(untrack(() => song.bpm));
	let steps = $state(
		untrack(() => song.progression.map((step) => ({ chord: step.chord, duration: step.duration })))
	);
	let drone = $state(
		untrack(() => ({
			note: song.drone?.note ?? '',
			preset: song.drone?.preset ?? ('custom' as 'tonic' | 'tritone' | 'custom')
		}))
	);

	const payload = $derived({
		key,
		mode,
		bpm,
		progression: steps.map((step) => ({ chord: step.chord, duration: step.duration })),
		drone: drone.note ? drone : null
	});

	let baseline = $state('');
	$effect(() => {
		// Set once per song, then only when a save lands.
		if (!baseline) baseline = JSON.stringify(payload);
	});
	const dirty = $derived(baseline !== '' && JSON.stringify(payload) !== baseline);
	let saving = $state(false);

	const tonic = $derived(parseNote(key) ?? { letter: 3, acc: 0 });
	const modeName = $derived(findMode(mode)?.name ?? mode);
	const palette = $derived(diatonicTriads(tonic, mode));
	const last = $derived(steps.length ? chordFromSymbol(steps[steps.length - 1].chord) : null);
	const mediantFrom = $derived(last?.root ?? tonic);
	const mediants = $derived(chromaticMediants(mediantFrom));
	const numbered = $derived(renumber(steps));
	const bars = $derived(totalBars(steps));
	const droneNote = $derived(drone.note ? parseNote(drone.note) : null);
	const primaryModes = MODES.filter((m) => m.primary);
	const moreModes = MODES.filter((m) => !m.primary);

	const staffChords = $derived(
		numbered
			.map((step) => {
				const chord = chordFromSymbol(step.chord);
				return chord
					? { bar: step.bar, label: label(chord), notes: chord.notes }
					: { bar: step.bar, label: step.chord, notes: [] };
			})
			.filter((chord) => chord.notes.length > 0)
	);

	/** Screen spelling: real ♭ and ♯ glyphs, quality suffix left alone. */
	function label(chord: Chord): string {
		return prettyNote(chord.root) + chord.symbol.slice(noteName(chord.root).length);
	}

	function add(chord: Chord) {
		if (!chord.symbol) return;
		steps = [...steps, { chord: chord.symbol, duration: 4 }];
	}

	function remove(index: number) {
		steps = steps.filter((_, i) => i !== index);
	}

	function move(index: number, by: number) {
		const target = index + by;
		if (target < 0 || target >= steps.length) return;
		const next = [...steps];
		[next[index], next[target]] = [next[target], next[index]];
		steps = next;
	}

	function setDuration(index: number, duration: number) {
		steps = steps.map((step, i) => (i === index ? { ...step, duration } : step));
	}

	function setDrone(preset: 'tonic' | 'tritone' | 'off') {
		if (preset === 'off') drone = { note: '', preset: 'custom' };
		else if (preset === 'tonic') drone = { note: noteName(tonic), preset: 'tonic' };
		else drone = { note: noteName(tritoneAbove(tonic)), preset: 'tritone' };
	}

	// A pedal set from the key follows the key when it changes.
	$effect(() => {
		const wanted =
			drone.preset === 'tonic'
				? noteName(tonic)
				: drone.preset === 'tritone'
					? noteName(tritoneAbove(tonic))
					: null;
		if (wanted && wanted !== drone.note) drone = { ...drone, note: wanted };
	});

	const gridText = $derived(buildGrid());

	function buildGrid(): string {
		const lines: string[] = [];
		lines.push(`${song.title} — ${key} ${modeName.toLowerCase()} · ${bpm} BPM · ${bars} bars`);
		if (droneNote) {
			const kind = drone.preset === 'tritone' ? 'tritone pedal' : 'pedal';
			lines.push(`drone ${noteName(droneNote)} (${kind}, bars 1–${bars || 1})`);
		}
		lines.push('');
		lines.push('BAR   CHORD   NOTES');
		for (const step of numbered) {
			const chord = chordFromSymbol(step.chord);
			const notes = chord ? chord.notes.map(noteName).join(' ') : '?';
			lines.push(`${String(step.bar).padEnd(6)}${step.chord.padEnd(8)}${notes}`);
		}
		return lines.join('\n');
	}

	let copied = $state(false);
	async function copyGrid() {
		try {
			await navigator.clipboard.writeText(gridText);
			copied = true;
			setTimeout(() => (copied = false), 1500);
		} catch {
			copied = false;
		}
	}
</script>

<div class="page-head">
	<p class="eyebrow">Progression</p>
	<div class="title-row">
		<h1>{song.title}</h1>
		<form
			method="post"
			action="?/save"
			use:enhance={() => {
				saving = true;
				return async ({ result, update }) => {
					saving = false;
					if (result.type === 'success') baseline = JSON.stringify(payload);
					await update({ reset: false });
				};
			}}
		>
			<input type="hidden" name="payload" value={JSON.stringify(payload)} />
			<button type="submit" disabled={!dirty || saving}>
				{saving ? 'Saving' : dirty ? 'Save' : 'Saved'}
			</button>
		</form>
	</div>
	<p class="lede">
		<span class="print-only">{key} {modeName.toLowerCase()} · {bpm} BPM · </span>{bars} bar{bars ===
		1
			? ''
			: 's'} · {steps.length} chord{steps.length === 1 ? '' : 's'}<span class="no-print">
			· written to <code>{file}</code></span
		>
	</p>
</div>

{#if form?.message}
	<div class="form-error">
		<p>{form.message}</p>
		{#if form.issues?.length}
			<ul>
				{#each form.issues as issue (issue.field + issue.message)}
					<li><span class="field">{issue.field}</span> — {issue.message}</li>
				{/each}
			</ul>
		{/if}
	</div>
{/if}

<section class="controls panel no-print">
	<div class="control">
		<span class="control-label" id="key-label">Key</span>
		<select bind:value={key} aria-labelledby="key-label">
			{#each KEYS as k (k)}
				<option value={k}>{k}</option>
			{/each}
		</select>
	</div>

	<div class="control modes">
		<span class="control-label">Mode</span>
		<div class="chips">
			{#each primaryModes as m (m.id)}
				<button
					class="chip"
					class:on={mode === m.id}
					aria-pressed={mode === m.id}
					onclick={() => (mode = m.id)}>{m.name}</button
				>
			{/each}
			<details class="more">
				<summary>more</summary>
				<div class="chips">
					{#each moreModes as m (m.id)}
						<button
							class="chip"
							class:on={mode === m.id}
							aria-pressed={mode === m.id}
							onclick={() => (mode = m.id)}>{m.name}</button
						>
					{/each}
				</div>
			</details>
		</div>
	</div>

	<div class="control">
		<span class="control-label" id="bpm-label">BPM</span>
		<input type="number" min="20" max="300" bind:value={bpm} aria-labelledby="bpm-label" />
		<div class="chips tempo">
			{#each TEMPO_BANDS as band (band.label)}
				<button
					class="chip"
					class:on={bpm >= band.low && bpm <= band.high}
					onclick={() => (bpm = band.pick)}>{band.low}–{band.high} {band.label}</button
				>
			{/each}
		</div>
	</div>
</section>

<section class="block no-print">
	<h2>{modeName} in {prettyNote(tonic)}</h2>
	<div class="palette">
		{#each palette as chord (chord.numeral)}
			<button
				class="chord"
				class:flat-two={chord.tag === 'flat-two'}
				class:passing={chord.tag === 'passing'}
				disabled={!chord.symbol}
				onclick={() => add(chord)}
				title={chord.symbol
					? `${qualityLabel(chord.quality)} — ${chord.notes.map(noteName).join(' ')}`
					: `${chord.notes.map(noteName).join(' ')} — no plain triad name, not placeable`}
			>
				<span class="numeral">{chord.numeral}</span>
				<span class="symbol">{chord.symbol ? label(chord) : '—'}</span>
				{#if chord.tag === 'passing'}<span class="note">passing</span>
				{:else if chord.tag === 'flat-two'}<span class="note">the ♭II</span>
				{:else if !chord.symbol}<span class="note">no name</span>{/if}
			</button>
		{/each}
	</div>
	{#if mode === 'phrygian'}
		<p class="dim hint">The ♭II is the reason you are in Phrygian. Land on it, don't pass through.</p>
	{:else}
		<p class="dim hint">
			Diminished and augmented chords are here, but they are passing colour — they want somewhere to
			go.
		</p>
	{/if}
</section>

<section class="block mediants no-print">
	<div class="mediants-head">
		<h2>Wrong in a good way</h2>
		<p class="dim">
			From {last ? label(last) : prettyNote(tonic)} — major triads a third away. One shared tone,
			no modulation, all menace.
		</p>
	</div>
	<div class="palette">
		{#each mediants as chord (chord.move)}
			<button
				class="chord mediant"
				onclick={() => add(chord)}
				title={chord.notes.map(noteName).join(' ')}
			>
				<span class="numeral">{chord.move}</span>
				<span class="symbol">{label(chord)}</span>
				<span class="note">{chord.notes.map(noteName).join(' ')}</span>
			</button>
		{/each}
	</div>
</section>

<section class="block">
	<h2>Bars</h2>
	{#if steps.length === 0}
		<p class="empty">No chords yet. Pick one above and it lands here.</p>
	{:else}
		<div class="timeline-scroll">
			<div
				class="timeline"
				style="grid-template-columns: {numbered
					.map((step) => `minmax(10.5rem, ${step.duration}fr)`)
					.join(' ')}"
			>
				{#each numbered as step, i (i)}
					<div class="slot">
						<div class="slot-head">
							<span class="slot-symbol"
								>{chordFromSymbol(step.chord) ? label(chordFromSymbol(step.chord)!) : step.chord}</span
							>
							<span class="slot-bars"
								>{step.bar}–{step.bar + step.duration - 1}</span
							>
						</div>
						<div class="slot-controls no-print">
							<button
								class="tiny"
								onclick={() => move(i, -1)}
								disabled={i === 0}
								aria-label="Move {step.chord} earlier">‹</button
							>
							<select
								value={step.duration}
								onchange={(event) => setDuration(i, Number(event.currentTarget.value))}
								aria-label="Bars for {step.chord}"
							>
								{#each DURATIONS as duration (duration)}
									<option value={duration}>{duration} bar{duration === 1 ? '' : 's'}</option>
								{/each}
							</select>
							<button
								class="tiny"
								onclick={() => move(i, 1)}
								disabled={i === steps.length - 1}
								aria-label="Move {step.chord} later">›</button
							>
							<button
								class="tiny remove"
								onclick={() => remove(i)}
								aria-label="Remove {step.chord}">✕</button
							>
						</div>
					</div>
				{/each}
			</div>

			<div class="drone-lane" class:on={!!droneNote}>
				<span class="drone-name">
					{#if droneNote}
						Drone {prettyNote(droneNote)}
						{drone.preset === 'tritone' ? '· tritone pedal' : drone.preset === 'tonic' ? '· tonic' : ''}
						— held bars 1–{bars}
					{:else}
						No drone
					{/if}
				</span>
			</div>
		</div>
	{/if}

	<div class="drone-controls no-print">
		<span class="control-label">Drone</span>
		<button class="chip" class:on={drone.preset === 'tonic' && !!drone.note} onclick={() => setDrone('tonic')}
			>Tonic</button
		>
		<button
			class="chip"
			class:on={drone.preset === 'tritone' && !!drone.note}
			onclick={() => setDrone('tritone')}>Tritone pedal</button
		>
		<button class="chip" class:on={!drone.note} onclick={() => setDrone('off')}>Off</button>
		<select
			value={drone.note}
			onchange={(event) => (drone = { note: event.currentTarget.value, preset: 'custom' })}
			aria-label="Drone note"
		>
			<option value="">—</option>
			{#each KEYS as k (k)}
				<option value={k}>{k}</option>
			{/each}
		</select>
	</div>
</section>

<section class="block">
	<h2>Staff</h2>
	{#if staffChords.length === 0}
		<p class="empty">Nothing to draw yet.</p>
	{:else}
		<Staff
			chords={staffChords}
			drone={droneNote}
			droneLabel={drone.preset === 'tritone' ? ' — tritone pedal' : ''}
		/>
		<p class="dim hint">
			Root position, treble staff, middle C at the first ledger line. Every accidental is spelled out
			— there is no key signature to read past.
		</p>
	{/if}
</section>

<section class="block">
	<div class="grid-head">
		<h2>Note grid</h2>
		<button class="no-print" onclick={copyGrid}>{copied ? 'Copied' : 'Copy'}</button>
	</div>
	<pre class="note-grid">{gridText}</pre>
	<p class="dim hint">For punching into a hardware sequencer by hand.</p>
</section>

<style>
	.title-row {
		display: flex;
		align-items: baseline;
		justify-content: space-between;
		gap: 1rem;
	}

	.controls {
		display: flex;
		flex-wrap: wrap;
		gap: 1.25rem 2rem;
		align-items: start;
		margin-bottom: 2rem;
	}

	.control {
		display: flex;
		flex-direction: column;
		gap: 0.35rem;
	}

	.control input[type='number'] {
		width: 7rem;
	}

	.control-label {
		font-size: 0.72rem;
		letter-spacing: 0.14em;
		text-transform: uppercase;
		color: var(--amber-dim);
	}

	.chips {
		display: flex;
		flex-wrap: wrap;
		gap: 0.35rem;
		align-items: center;
	}

	.chip {
		font-family: var(--font-mono);
		text-transform: none;
		letter-spacing: 0.02em;
		font-size: 0.75rem;
		padding: 0.2rem 0.55rem;
		background: none;
		color: var(--amber-dim);
	}

	.chip.on {
		color: var(--cyan-bright);
		border-color: var(--cyan);
	}

	.tempo {
		margin-top: 0.4rem;
	}

	.more {
		display: inline-block;
	}

	.more summary {
		cursor: pointer;
		font-size: 0.75rem;
		color: var(--amber-dim);
		border: var(--rule);
		border-radius: var(--radius);
		padding: 0.2rem 0.55rem;
		list-style: none;
	}

	.more summary::-webkit-details-marker {
		display: none;
	}

	.more[open] summary {
		color: var(--cyan-bright);
		border-color: var(--cyan);
	}

	.more .chips {
		margin-top: 0.4rem;
	}

	.block {
		margin-bottom: 2rem;
	}

	.block h2 {
		border-bottom: var(--rule);
		padding-bottom: 0.3rem;
		margin-bottom: 0.9rem;
	}

	.hint {
		font-size: 0.8rem;
		margin: 0.7rem 0 0;
	}

	.palette {
		display: flex;
		flex-wrap: wrap;
		gap: 0.5rem;
	}

	.chord {
		display: flex;
		flex-direction: column;
		align-items: start;
		gap: 0.15rem;
		min-width: 5.5rem;
		padding: 0.5rem 0.7rem;
		background: var(--bg-panel);
		text-transform: none;
		letter-spacing: 0;
	}

	.chord .numeral {
		font-family: var(--font-mono);
		font-size: 0.72rem;
		letter-spacing: 0.1em;
		color: var(--amber-dim);
	}

	.chord .symbol {
		font-family: var(--font-display);
		font-stretch: condensed;
		letter-spacing: 0.03em;
		font-size: 1.2rem;
		color: var(--amber);
	}

	.chord .note {
		font-family: var(--font-mono);
		font-size: 0.68rem;
		color: var(--amber-dim);
		letter-spacing: 0.04em;
	}

	.chord:hover:not(:disabled),
	.chord:focus-visible {
		border-color: var(--cyan);
	}

	.chord:disabled {
		opacity: 0.45;
		cursor: not-allowed;
	}

	.chord.passing {
		border-style: dashed;
	}

	.chord.flat-two {
		border-color: var(--cyan);
		border-left-width: 3px;
	}

	.chord.flat-two .symbol {
		color: var(--cyan-bright);
	}

	.mediants {
		border: 1px solid var(--cyan);
		border-left-width: 4px;
		background: color-mix(in srgb, var(--cyan) 7%, var(--bg-panel));
		padding: 1rem 1.15rem 1.15rem;
		border-radius: var(--radius);
	}

	.mediants h2 {
		color: var(--cyan-bright);
		border-bottom: 0;
		padding-bottom: 0;
		margin-bottom: 0.15rem;
		font-size: 1.45rem;
	}

	.mediants-head p {
		font-size: 0.85rem;
		margin-bottom: 0.9rem;
	}

	.chord.mediant {
		min-width: 7rem;
		border-color: var(--cyan);
	}

	.chord.mediant .numeral {
		color: var(--cyan-bright);
	}

	.timeline-scroll {
		overflow-x: auto;
	}

	.timeline {
		display: grid;
		gap: 0.35rem;
		min-width: 100%;
	}

	.slot {
		border: var(--rule);
		border-left: 2px solid var(--amber-dim);
		background: var(--bg-panel);
		border-radius: var(--radius);
		padding: 0.55rem 0.6rem;
		display: flex;
		flex-direction: column;
		gap: 0.5rem;
		min-width: 0;
	}

	.slot-head {
		display: flex;
		flex-direction: column;
		gap: 0.1rem;
	}

	.slot-symbol {
		font-family: var(--font-display);
		font-stretch: condensed;
		letter-spacing: 0.03em;
		font-size: 1.15rem;
	}

	.slot-bars {
		font-size: 0.72rem;
		color: var(--amber-dim);
		letter-spacing: 0.08em;
	}

	.slot-controls {
		display: flex;
		flex-wrap: wrap;
		gap: 0.25rem;
		align-items: center;
	}

	.slot-controls select {
		font-size: 0.72rem;
		padding: 0.15rem 0.2rem;
		flex: 1 1 4rem;
		min-width: 4rem;
	}

	.tiny {
		padding: 0.1rem 0.3rem;
		font-size: 0.75rem;
		font-family: var(--font-mono);
		letter-spacing: 0;
	}

	.tiny.remove:hover {
		color: var(--oxblood-bright);
		border-color: var(--oxblood);
	}

	.drone-lane {
		margin-top: 0.35rem;
		border: 1px dashed var(--amber-line);
		border-radius: var(--radius);
		padding: 0.4rem 0.6rem;
		font-size: 0.75rem;
		letter-spacing: 0.08em;
		text-transform: uppercase;
		color: var(--amber-dim);
	}

	.drone-lane.on {
		border-style: solid;
		border-color: var(--cyan);
		color: var(--cyan-bright);
	}

	.drone-controls {
		display: flex;
		flex-wrap: wrap;
		align-items: center;
		gap: 0.4rem;
		margin-top: 0.9rem;
	}

	.grid-head {
		display: flex;
		align-items: baseline;
		justify-content: space-between;
		gap: 1rem;
		border-bottom: var(--rule);
		padding-bottom: 0.3rem;
		margin-bottom: 0.9rem;
	}

	.grid-head h2 {
		border: 0;
		padding: 0;
		margin: 0;
	}

	.note-grid {
		margin: 0;
		font-size: 0.82rem;
		line-height: 1.5;
	}

	.form-error {
		border: 1px solid var(--oxblood-bright);
		border-left-width: 4px;
		background: color-mix(in srgb, var(--oxblood) 18%, var(--bg));
		padding: 0.7rem 1rem;
		border-radius: var(--radius);
		margin-bottom: 1.5rem;
		color: var(--oxblood-bright);
	}

	.form-error p {
		margin: 0;
	}

	.form-error ul {
		margin: 0.4rem 0 0;
		padding-left: 1.1rem;
		font-size: 0.85rem;
	}

	.field {
		color: var(--oxblood-bright);
	}

	@media print {
		.mediants {
			background: #fff;
		}

		/* Half-letter is too narrow for one column per chord. */
		.timeline {
			grid-template-columns: 1fr 1fr !important;
		}

		.slot {
			break-inside: avoid;
		}
	}

	@media (max-width: 48rem) {
		.controls {
			gap: 1rem;
		}
	}
</style>
