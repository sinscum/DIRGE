<script lang="ts">
	import ErrorPanel from '$lib/components/ErrorPanel.svelte';
	import type { PageData } from './$types';

	let { data }: { data: PageData } = $props();

	const fxChain = $derived(data.recipe.depends_on.fx_chain);
</script>

<svelte:head>
	<title>{data.recipe.name} — {data.device.name} — Dirge</title>
</svelte:head>

<ErrorPanel errors={data.errors} />

<nav class="crumbs no-print" aria-label="Breadcrumb">
	<a href="/">Rack</a> / <a href="/devices/{data.device.id}">{data.device.name}</a> / recipes
</nav>

<div class="page-head">
	<p class="eyebrow">Patch · {data.recipe.category}</p>
	<div class="title-row">
		<h1>{data.recipe.name}</h1>
		<button class="no-print" onclick={() => window.print()}>Print</button>
	</div>
	<p class="lede">
		<span class="print-only">{data.device.name} · </span>{data.recipe.category}
		{#if data.recipe.key_agnostic}· key agnostic{/if}
		· <code>{data.recipe.id}</code>
	</p>
</div>

{#if data.html}
	<div class="prose panel notes">
		<!-- eslint-disable-next-line svelte/no-at-html-tags -->
		{@html data.html}
	</div>
{/if}

<section class="block">
	<h2>Parameters</h2>
	{#if data.groups.length === 0}
		<p class="empty">
			No parameters in <code>{data.file}</code>. Add a <code>params:</code> list to the frontmatter.
		</p>
	{:else}
		<div class="param-groups">
			{#each data.groups as group (group.section)}
				<table class="params">
					<caption>{group.section}</caption>
					<thead>
						<tr>
							<th scope="col">Control</th>
							<th scope="col">Value</th>
						</tr>
					</thead>
					<tbody>
						{#each group.params as param (param.control + param.value)}
							<tr>
								<th scope="row">{param.control}</th>
								<td class="value">{param.value}</td>
							</tr>
						{/each}
					</tbody>
				</table>
			{/each}
		</div>
	{/if}
</section>

{#if data.recipe.depends_on.clock || fxChain.length || data.recipe.audio_ref}
	<section class="block">
		<h2>Depends on</h2>
		<dl class="meta-list panel">
			{#if data.recipe.depends_on.clock}
				<dt>Clock</dt>
				<dd>{data.recipe.depends_on.clock}</dd>
			{/if}
			{#each fxChain as fx, i (fx)}
				<dt>{i === 0 ? 'FX chain' : ''}</dt>
				<dd>{fx}</dd>
			{/each}
			{#if data.recipe.audio_ref}
				<dt>Audio</dt>
				<dd><a href={data.recipe.audio_ref}>{data.recipe.audio_ref}</a></dd>
			{/if}
		</dl>
	</section>
{/if}

<section class="block">
	<h2>Used in</h2>
	{#if data.uses.length === 0}
		<p class="empty">
			No song points a layer at <code>{data.recipe.id}</code> yet.
		</p>
	{:else}
		<table class="uses">
			<thead>
				<tr>
					<th scope="col">Song</th>
					<th scope="col">Key</th>
					<th scope="col">BPM</th>
					<th scope="col">Slot</th>
					<th scope="col">Status</th>
				</tr>
			</thead>
			<tbody>
				{#each data.uses as use (use.id)}
					<tr>
						<th scope="row">{use.title}</th>
						<td>{use.key} {use.mode}</td>
						<td class="num">{use.bpm}</td>
						<td>{use.slots.join(', ')}</td>
						<td class="dim">{use.status}</td>
					</tr>
				{/each}
			</tbody>
		</table>
	{/if}
</section>

<p class="source dim no-print">Source: <code>{data.file}</code></p>

<style>
	.crumbs {
		font-size: 0.78rem;
		letter-spacing: 0.1em;
		text-transform: uppercase;
		color: var(--amber-dim);
		margin-bottom: 1rem;
	}

	.crumbs a {
		color: var(--amber-dim);
		border-bottom-color: var(--amber-line-soft);
	}

	.title-row {
		display: flex;
		align-items: baseline;
		justify-content: space-between;
		gap: 1rem;
	}

	.notes {
		margin-bottom: 2rem;
	}

	.notes :global(p:last-child) {
		margin-bottom: 0;
	}

	.block {
		margin-bottom: 2rem;
	}

	.block h2 {
		border-bottom: var(--rule);
		padding-bottom: 0.3rem;
		margin-bottom: 0.9rem;
	}

	.param-groups {
		display: grid;
		gap: 1rem;
		grid-template-columns: repeat(auto-fill, minmax(16rem, 1fr));
	}

	.params {
		border: var(--rule);
		background: var(--bg-panel);
		margin: 0;
	}

	.params caption {
		font-family: var(--font-display);
		font-stretch: condensed;
		text-transform: uppercase;
		letter-spacing: 0.15em;
		font-size: 0.75rem;
		text-align: left;
		color: var(--cyan-bright);
		padding: 0.4rem 0.6rem 0.3rem;
		border-bottom: var(--rule);
	}

	.params th[scope='row'] {
		font-family: var(--font-mono);
		text-transform: none;
		letter-spacing: 0;
		font-weight: 400;
		color: var(--amber-dim);
		border-bottom: 1px solid var(--amber-line-soft);
	}

	.params tbody tr:last-child th,
	.params tbody tr:last-child td {
		border-bottom: 0;
	}

	.value,
	.num {
		font-variant-numeric: tabular-nums;
		text-align: right;
		white-space: nowrap;
	}

	.uses {
		border: var(--rule);
		background: var(--bg-panel);
	}

	.uses th[scope='row'] {
		font-family: var(--font-mono);
		text-transform: none;
		letter-spacing: 0;
		font-weight: 400;
		color: var(--amber);
	}

	.source {
		font-size: 0.75rem;
	}

	@media print {
		.params caption {
			color: #000;
			border-bottom: 0.5pt solid #000;
		}

		.param-groups {
			grid-template-columns: 1fr 1fr;
			gap: 0.15in;
		}

		.params,
		.uses {
			break-inside: avoid;
			border: 0.5pt solid #000;
		}
	}
</style>
