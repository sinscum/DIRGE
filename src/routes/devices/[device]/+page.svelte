<script lang="ts">
	import ErrorPanel from '$lib/components/ErrorPanel.svelte';
	import type { PageData } from './$types';

	let { data }: { data: PageData } = $props();

	const categories = $derived([...new Set(data.recipes.map((r) => r.category))].sort());
	let filter = $state<string>('all');
	const shown = $derived(
		filter === 'all' ? data.recipes : data.recipes.filter((r) => r.category === filter)
	);

	// Category options reset when you walk to another device.
	$effect(() => {
		data.device.id;
		filter = 'all';
	});

	const io = $derived(Object.entries(data.device.io));
</script>

<svelte:head>
	<title>{data.device.name} — Dirge</title>
</svelte:head>

<ErrorPanel errors={data.errors} />

<div class="page-head">
	<p class="eyebrow">Rack · {data.device.type}</p>
	<div class="title-row">
		<h1>{data.device.name}</h1>
		<button class="no-print" onclick={() => window.print()}>Print</button>
	</div>
	{#if data.device.role}<p class="lede">{data.device.role}</p>{/if}
</div>

<dl class="meta-list vitals panel">
	{#if data.device.firmware}
		<dt>Firmware</dt>
		<dd>{data.device.firmware}</dd>
	{/if}
	{#if data.device.polyphony}
		<dt>Voices</dt>
		<dd>{data.device.polyphony}</dd>
	{/if}
	{#if data.device.midi_channel}
		<dt>MIDI ch</dt>
		<dd>{data.device.midi_channel}</dd>
	{/if}
	{#if data.device.engines.length}
		<dt>Engines</dt>
		<dd>{data.device.engines.join(', ')}</dd>
	{/if}
	{#each io as [name, value] (name)}
		<dt>{name.replace(/_/g, ' ')}</dt>
		<dd>{value}</dd>
	{/each}
	{#if data.device.tags.length}
		<dt>Tags</dt>
		<dd>{data.device.tags.join(', ')}</dd>
	{/if}
	{#if data.device.manual_url}
		<dt>Manual</dt>
		<dd><a href={data.device.manual_url}>{data.device.manual_url}</a></dd>
	{/if}
</dl>

{#each data.sections as section (section.slug)}
	<section class="body-section" id={section.slug}>
		<h2>{section.title}</h2>
		{#if section.missing}
			<p class="empty no-print">
				No <strong>{section.title}</strong> section yet. {section.hint} Add
				<code>## {section.title}</code> to <code>{data.file}</code>.
			</p>
		{:else}
			<div class="prose">
				<!-- eslint-disable-next-line svelte/no-at-html-tags -->
				{@html section.html}
			</div>
		{/if}
	</section>
{/each}

<section class="body-section" id="recipes">
	<div class="recipes-head">
		<h2>Recipes</h2>
		{#if categories.length > 1}
			<div class="filters no-print" role="group" aria-label="Filter recipes by category">
				<button
					class="chip"
					class:on={filter === 'all'}
					aria-pressed={filter === 'all'}
					onclick={() => (filter = 'all')}>all</button
				>
				{#each categories as category (category)}
					<button
						class="chip"
						class:on={filter === category}
						aria-pressed={filter === category}
						onclick={() => (filter = category)}>{category}</button
					>
				{/each}
			</div>
		{/if}
	</div>

	{#if data.recipes.length === 0}
		<p class="empty">
			No recipes yet. Add a file under <code>data/devices/{data.device.id}/recipes/</code>.
		</p>
	{:else}
		<div class="grid">
			{#each shown as recipe (recipe.id)}
				<a class="recipe" href="/devices/{data.device.id}/recipes/{recipe.id}">
					<span class="row">
						<span class="name">{recipe.name}</span>
						<span class="tag">{recipe.category}</span>
					</span>
					{#if recipe.summary}<span class="summary">{recipe.summary}</span>{/if}
					<span class="foot dim">
						{recipe.paramCount} param{recipe.paramCount === 1 ? '' : 's'}
						{#if recipe.key_agnostic} · key agnostic{/if}
					</span>
				</a>
			{/each}
		</div>
	{/if}
</section>

<style>
	.title-row {
		display: flex;
		align-items: baseline;
		justify-content: space-between;
		gap: 1rem;
	}

	.vitals {
		margin-bottom: 2rem;
	}

	.body-section {
		margin-bottom: 2rem;
		scroll-margin-top: 4.5rem;
	}

	.body-section h2 {
		color: var(--amber);
		border-bottom: var(--rule);
		padding-bottom: 0.3rem;
		margin-bottom: 0.9rem;
	}

	.prose :global(h3) {
		color: var(--cyan-bright);
		margin-top: 1.25rem;
	}

	.prose :global(ul) {
		padding-left: 1.1rem;
	}

	.prose :global(li) {
		margin-bottom: 0.2rem;
	}

	.prose :global(table th) {
		white-space: nowrap;
	}

	.recipes-head {
		display: flex;
		align-items: baseline;
		justify-content: space-between;
		gap: 1rem;
		border-bottom: var(--rule);
		padding-bottom: 0.3rem;
		margin-bottom: 0.9rem;
	}

	.recipes-head h2 {
		border: 0;
		padding: 0;
		margin: 0;
	}

	.filters {
		display: flex;
		flex-wrap: wrap;
		gap: 0.35rem;
	}

	.chip {
		font-family: var(--font-mono);
		text-transform: lowercase;
		letter-spacing: 0.04em;
		font-size: 0.75rem;
		padding: 0.15rem 0.5rem;
		background: none;
		color: var(--amber-dim);
	}

	.chip.on {
		color: var(--cyan-bright);
		border-color: var(--cyan);
	}

	.recipe {
		display: flex;
		flex-direction: column;
		gap: 0.4rem;
		border: var(--rule);
		background: var(--bg-panel);
		padding: 0.8rem 0.9rem;
		border-radius: var(--radius);
	}

	.recipe:hover,
	.recipe:focus-visible {
		border-color: var(--cyan);
		color: var(--amber);
	}

	.row {
		display: flex;
		align-items: center;
		justify-content: space-between;
		gap: 0.5rem;
	}

	.name {
		font-family: var(--font-display);
		font-stretch: condensed;
		text-transform: uppercase;
		letter-spacing: 0.05em;
		font-size: 1.05rem;
	}

	.summary {
		font-size: 0.8rem;
		color: var(--amber-dim);
		flex: 1;
	}

	.foot {
		font-size: 0.72rem;
		text-transform: uppercase;
		letter-spacing: 0.08em;
	}
</style>
