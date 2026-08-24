<script lang="ts">
	import ErrorPanel from '$lib/components/ErrorPanel.svelte';
	import type { PageData } from './$types';

	let { data }: { data: PageData } = $props();
</script>

<svelte:head>
	<title>Rack — Dirge</title>
</svelte:head>

<div class="page-head">
	<p class="eyebrow">Rack</p>
	<h1>Hardware reference</h1>
	<p class="lede">
		{data.deviceCount} device{data.deviceCount === 1 ? '' : 's'} on disk. Patch recipes, shortcuts,
		and the fixes that took an hour to find the first time.
	</p>
</div>

<ErrorPanel errors={data.errors} />

{#if data.groups.length === 0}
	<p class="empty">
		No devices yet. Add <code>data/devices/&lt;id&gt;/device.md</code> and refresh.
	</p>
{/if}

{#each data.groups as group (group.type)}
	<section class="group">
		<h2>{group.type}</h2>
		<div class="grid">
			{#each group.devices as device (device.id)}
				<a class="card" href="/devices/{device.id}">
					<span class="name">{device.name}</span>
					{#if device.role}<span class="role">{device.role}</span>{/if}
					<span class="foot">
						<span class="count"
							>{device.recipeCount} recipe{device.recipeCount === 1 ? '' : 's'}</span
						>
						{#if device.midi_channel}<span class="dim">MIDI ch {device.midi_channel}</span>{/if}
					</span>
				</a>
			{/each}
		</div>
	</section>
{/each}

<style>
	.group {
		margin-bottom: 2.25rem;
	}

	.group h2 {
		color: var(--amber-dim);
		font-size: 0.85rem;
		letter-spacing: 0.22em;
		border-bottom: var(--rule);
		padding-bottom: 0.35rem;
		margin-bottom: 1rem;
	}

	.card {
		display: flex;
		flex-direction: column;
		gap: 0.45rem;
		border: var(--rule);
		border-left: 2px solid var(--amber-dim);
		background: var(--bg-panel);
		padding: 0.9rem 1rem;
		border-radius: var(--radius);
		min-height: 8rem;
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

	.role {
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
</style>
