<script lang="ts">
	import ErrorPanel from '$lib/components/ErrorPanel.svelte';
	import { KEYS, MODES, TEMPO_BANDS } from '$lib/music';
	import type { ActionData, PageData } from './$types';

	let { data, form }: { data: PageData; form: ActionData } = $props();

	const primary = MODES.filter((m) => m.primary);
	const more = MODES.filter((m) => !m.primary);
</script>

<svelte:head>
	<title>Forge — Dirge</title>
</svelte:head>

<div class="page-head">
	<p class="eyebrow">Forge</p>
	<h1>Songs</h1>
	<p class="lede">
		Progressions in modes that already sound like the genre. Everything here is stored in
		<code>data/songs</code> — the builder writes markdown, nothing else.
	</p>
</div>

<ErrorPanel errors={data.errors} />

{#if form?.message}
	<p class="form-error">{form.message}</p>
{/if}

{#if data.songs.length === 0}
	<p class="empty">No songs yet. Start one below.</p>
{:else}
	<div class="grid">
		{#each data.songs as song (song.id)}
			<a class="card" href="/forge/{song.id}">
				<span class="name">{song.title}</span>
				<span class="spec">{song.key} {song.mode} · {song.bpm} BPM</span>
				<span class="foot">
					<span class="count">{song.bars} bar{song.bars === 1 ? '' : 's'}</span>
					<span class="dim">{song.status}</span>
				</span>
			</a>
		{/each}
	</div>
{/if}

<section class="new">
	<h2>Start a song</h2>
	<form method="post" action="?/create">
		<label>
			<span>Title</span>
			<input name="title" type="text" required placeholder="Black Tide" />
		</label>
		<label>
			<span>Key</span>
			<select name="key">
				{#each KEYS as key (key)}
					<option value={key} selected={key === 'F'}>{key}</option>
				{/each}
			</select>
		</label>
		<label>
			<span>Mode</span>
			<select name="mode">
				{#each primary as mode (mode.id)}
					<option value={mode.id} selected={mode.id === 'phrygian'}>{mode.name}</option>
				{/each}
				<optgroup label="more">
					{#each more as mode (mode.id)}
						<option value={mode.id}>{mode.name}</option>
					{/each}
				</optgroup>
			</select>
		</label>
		<label>
			<span>BPM</span>
			<input name="bpm" type="number" min="20" max="300" value="96" />
		</label>
		<button type="submit">Start song</button>
	</form>
	<p class="dim tempo">
		{#each TEMPO_BANDS as band, i (band.label)}{i > 0 ? ' · ' : ''}{band.low}–{band.high}
			{band.label}{/each}
	</p>
</section>

<style>
	.card {
		display: flex;
		flex-direction: column;
		gap: 0.45rem;
		border: var(--rule);
		border-left: 2px solid var(--amber-dim);
		background: var(--bg-panel);
		padding: 0.9rem 1rem;
		border-radius: var(--radius);
		min-height: 7rem;
	}

	.card:hover,
	.card:focus-visible {
		border-color: var(--cyan);
		border-left-color: var(--cyan);
		color: var(--amber);
	}

	.name {
		font-family: var(--font-display);
		font-stretch: condensed;
		text-transform: uppercase;
		letter-spacing: 0.05em;
		font-size: 1.15rem;
	}

	.spec {
		color: var(--amber-dim);
		font-size: 0.85rem;
		flex: 1;
	}

	.foot {
		display: flex;
		justify-content: space-between;
		gap: 0.5rem;
		font-size: 0.75rem;
		letter-spacing: 0.08em;
		text-transform: uppercase;
		border-top: 1px solid var(--amber-line-soft);
		padding-top: 0.45rem;
	}

	.count {
		color: var(--cyan-bright);
	}

	.new {
		margin-top: 2.5rem;
		border-top: var(--rule);
		padding-top: 1.25rem;
	}

	form {
		display: flex;
		flex-wrap: wrap;
		align-items: end;
		gap: 0.75rem;
	}

	label {
		display: flex;
		flex-direction: column;
		gap: 0.25rem;
		font-size: 0.72rem;
		letter-spacing: 0.12em;
		text-transform: uppercase;
		color: var(--amber-dim);
	}

	input[type='text'] {
		min-width: 14rem;
	}

	input[type='number'] {
		width: 6rem;
		font-family: var(--font-mono);
		font-size: 0.85rem;
		color: var(--amber);
		background: var(--bg-panel);
		border: var(--rule);
		border-radius: var(--radius);
		padding: 0.35rem 0.55rem;
	}

	.tempo {
		font-size: 0.78rem;
		margin-top: 0.6rem;
	}

	.form-error {
		border: 1px solid var(--oxblood-bright);
		border-left-width: 4px;
		color: var(--oxblood-bright);
		background: color-mix(in srgb, var(--oxblood) 18%, var(--bg));
		padding: 0.6rem 0.9rem;
		border-radius: var(--radius);
	}
</style>
