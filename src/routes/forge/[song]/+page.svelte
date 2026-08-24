<script lang="ts">
	import ErrorPanel from '$lib/components/ErrorPanel.svelte';
	import ProgressionBuilder from '$lib/components/ProgressionBuilder.svelte';
	import type { ActionData, PageData } from './$types';

	let { data, form }: { data: PageData; form: ActionData } = $props();
</script>

<svelte:head>
	<title>{data.song.title} — Forge — Dirge</title>
</svelte:head>

<ErrorPanel errors={data.errors} />

<nav class="crumbs no-print" aria-label="Breadcrumb">
	<a href="/forge">Forge</a> / songs
</nav>

{#key data.song.id}
	<ProgressionBuilder song={data.song} file={data.file} {form} />
{/key}

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
</style>
