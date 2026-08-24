<script lang="ts">
	import { prettyNote, type Note } from '$lib/music';

	type StaffChord = { bar: number; label: string; notes: Note[] };

	let {
		chords,
		drone = null,
		droneLabel = ''
	}: { chords: StaffChord[]; drone?: Note | null; droneLabel?: string } = $props();

	// Staff geometry. Positions are diatonic steps: p = letter + 7 * octave.
	// F5 (top line) = 38, E4 (bottom line) = 30, one step = half a space.
	const HALF = 4;
	const TOP = 42;
	const COL = 88;
	const PAD = 84;
	const TOP_LINE_P = 38;
	const BOTTOM_LINE_P = 30;
	const BAR_LABEL_Y = 104;
	const DRONE_Y = 132;

	function y(p: number): number {
		return TOP + (TOP_LINE_P - p) * HALF;
	}

	/** Root sits in the octave above middle C; the triad stacks up in letter steps from there. */
	function positions(notes: Note[]): number[] {
		if (!notes.length) return [];
		const rootLetter = notes[0].letter;
		return notes.map((note, i) => {
			// Follow the letters upward, so a triad never collapses onto itself.
			let letter = note.letter;
			while (letter < rootLetter + (i > 0 ? 1 : 0)) letter += 7;
			return letter + 28;
		});
	}

	function accidental(note: Note): string {
		if (note.acc === 0) return '';
		return note.acc > 0 ? '♯'.repeat(note.acc) : '♭'.repeat(-note.acc);
	}

	function ledgers(p: number): number[] {
		const out: number[] = [];
		for (let q = BOTTOM_LINE_P - 2; q >= p; q -= 2) out.push(q);
		for (let q = TOP_LINE_P + 2; q <= p; q += 2) out.push(q);
		return out;
	}

	const width = $derived(PAD + Math.max(chords.length, 1) * COL + 16);
	const height = $derived(drone ? 152 : 120);
	const staffLines = [38, 36, 34, 32, 30];
</script>

<div class="staff-wrap">
	<svg
		viewBox="0 0 {width} {height}"
		width={width}
		{height}
		role="img"
		aria-label="Staff view of the progression"
	>
		<!-- staff -->
		{#each staffLines as line (line)}
			<line x1={PAD - 16} y1={y(line)} x2={width - 8} y2={y(line)} class="rule" />
		{/each}
		<line x1={PAD - 16} y1={y(38)} x2={PAD - 16} y2={y(30)} class="rule" />
		<text x={8} y={y(34) + 4} class="clef">treble</text>

		{#each chords as chord, i (chord.bar + chord.label + i)}
			{@const x = PAD + i * COL + COL / 2}
			<text {x} y={22} class="symbol" text-anchor="middle">{chord.label}</text>
			<text {x} y={BAR_LABEL_Y} class="bar" text-anchor="middle">bar {chord.bar}</text>

			{#each positions(chord.notes) as p, n (p)}
				{#each ledgers(p) as q (q)}
					<line x1={x - 13} y1={y(q)} x2={x + 13} y2={y(q)} class="rule" />
				{/each}
				<ellipse
					cx={x}
					cy={y(p)}
					rx="6"
					ry="4.2"
					class="head"
					transform="rotate(-18 {x} {y(p)})"
				/>
				{#if accidental(chord.notes[n])}
					<text x={x - 13} y={y(p) + 4} class="acc" text-anchor="end"
						>{accidental(chord.notes[n])}</text
					>
				{/if}
			{/each}

			{#if i > 0}
				<line
					x1={PAD + i * COL}
					y1={y(38)}
					x2={PAD + i * COL}
					y2={y(30)}
					class="rule barline"
				/>
			{/if}
		{/each}

		{#if drone}
			<text x={8} y={DRONE_Y + 4} class="drone-label">drone</text>
			<line
				x1={PAD - 16}
				y1={DRONE_Y}
				x2={width - 8}
				y2={DRONE_Y}
				class="drone-line"
				stroke-dasharray="6 4"
			/>
			<ellipse cx={PAD} cy={DRONE_Y} rx="6" ry="4.2" class="head drone-head" />
			<text x={PAD + 14} y={DRONE_Y - 8} class="drone-note">{prettyNote(drone)}{droneLabel}</text>
		{/if}
	</svg>
</div>

<style>
	.staff-wrap {
		overflow-x: auto;
		border: var(--rule);
		background: var(--bg-panel);
		border-radius: var(--radius);
		padding: 0.5rem 0.25rem;
	}

	svg {
		display: block;
		max-width: none;
	}

	.rule {
		stroke: var(--amber-dim);
		stroke-width: 1;
	}

	.barline {
		stroke: var(--amber-line);
	}

	.head {
		fill: var(--amber);
	}

	.drone-head {
		fill: var(--cyan-bright);
	}

	.drone-line {
		stroke: var(--cyan);
		stroke-width: 1.5;
	}

	.symbol {
		fill: var(--amber);
		font-family: var(--font-display);
		font-stretch: condensed;
		font-size: 16px;
		letter-spacing: 0.04em;
	}

	.bar,
	.clef,
	.drone-label {
		fill: var(--amber-dim);
		font-family: var(--font-mono);
		font-size: 10px;
		letter-spacing: 0.1em;
		text-transform: uppercase;
	}

	.clef {
		letter-spacing: 0.14em;
	}

	.acc {
		fill: var(--amber);
		font-family: var(--font-mono);
		font-size: 13px;
	}

	.drone-note {
		fill: var(--cyan-bright);
		font-family: var(--font-mono);
		font-size: 12px;
	}

	@media print {
		.staff-wrap {
			border: 0.5pt solid #000;
			overflow: visible;
		}

		/* Scale the whole staff to the page instead of running off it. */
		svg {
			width: 100%;
			height: auto;
		}

		.rule,
		.drone-line {
			stroke: #000;
		}

		.head,
		.drone-head,
		.symbol,
		.bar,
		.clef,
		.acc,
		.drone-note,
		.drone-label {
			fill: #000;
		}
	}
</style>
