<script lang="ts">
	import { page } from '$app/state';
</script>

<svelte:head>
	<title>{page.status} — Dirge</title>
</svelte:head>

<div class="page-head">
	<p class="eyebrow">Error {page.status}</p>
	<h1>{page.status === 404 ? 'Not on disk' : 'Something broke'}</h1>
	<p class="lede">{page.error?.message}</p>
</div>

<p><a href="/">Back to the rack</a></p>
