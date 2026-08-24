<script lang="ts">
	import ErrorPanel from '$lib/components/ErrorPanel.svelte';
	import type { PageData } from './$types';

	let { data }: { data: PageData } = $props();
</script>

<svelte:head>
	<title>{data.query ? `${data.query} — search` : 'Search'} — Dirge</title>
</svelte:head>

<div class="page-head">
	<p class="eyebrow">Search</p>
	<h1>{data.query || 'Everything on disk'}</h1>
	<p class="lede">
		{#if data.query}
			{data.hits.length} hit{data.hits.length === 1 ? '' : 's'} across device names, recipe names, and
			troubleshooting symptoms.
		{:else}
			Type a device, a patch, or a symptom — "no sound", "clock", "choir".
		{/if}
	</p>
</div>

<ErrorPanel errors={data.errors} />

{#if data.query && data.hits.length === 0}
	<p class="empty">Nothing matched <code>{data.query}</code>.</p>
{/if}

<ol class="hits">
	{#each data.hits as hit (hit.kind + hit.href + hit.title)}
		<li>
			<a href={hit.href}>
				<span class="row">
					<span class="title">{hit.title}</span>
					<span class="tag">{hit.kind}</span>
				</span>
				<span class="context">{hit.context}</span>
				{#if hit.snippet}<span class="snippet">{hit.snippet}</span>{/if}
			</a>
		</li>
	{/each}
</ol>

<style>
	.hits {
		list-style: none;
		margin: 0;
		padding: 0;
		display: grid;
		gap: 0.5rem;
	}

	.hits a {
		display: flex;
		flex-direction: column;
		gap: 0.25rem;
		border: var(--rule);
		border-left: 2px solid var(--amber-dim);
		background: var(--bg-panel);
		padding: 0.7rem 0.9rem;
		border-radius: var(--radius);
	}

	.hits a:hover,
	.hits a:focus-visible {
		border-color: var(--cyan);
		color: var(--amber);
	}

	.row {
		display: flex;
		align-items: center;
		justify-content: space-between;
		gap: 0.6rem;
	}

	.title {
		font-family: var(--font-display);
		font-stretch: condensed;
		text-transform: uppercase;
		letter-spacing: 0.05em;
		font-size: 1.05rem;
	}

	.context {
		font-size: 0.75rem;
		text-transform: uppercase;
		letter-spacing: 0.1em;
		color: var(--amber-dim);
	}

	.snippet {
		font-size: 0.82rem;
		color: var(--amber-dim);
	}
</style>
