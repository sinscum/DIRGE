<script lang="ts">
	import '$lib/styles/app.css';
	import { page } from '$app/state';

	let { children } = $props();

	const nav = [
		{ href: '/', label: 'Rack' },
		{ href: '/forge', label: 'Forge' }
	];

	function current(href: string): 'page' | undefined {
		if (href === '/') {
			return page.url.pathname === '/' || page.url.pathname.startsWith('/devices')
				? 'page'
				: undefined;
		}
		return page.url.pathname.startsWith(href) ? 'page' : undefined;
	}
</script>

<header class="masthead no-print">
	<div class="masthead-inner">
		<a class="wordmark" href="/">Dirge<span class="caret">_</span></a>
		<nav class="nav" aria-label="Sections">
			{#each nav as item (item.href)}
				<a href={item.href} aria-current={current(item.href)}>{item.label}</a>
			{/each}
		</nav>
		<form class="search-form" action="/search" method="get" role="search">
			<label class="visually-hidden" for="q">Search</label>
			<input
				id="q"
				name="q"
				type="search"
				placeholder="devices, recipes, symptoms"
				value={page.url.pathname === '/search' ? (page.url.searchParams.get('q') ?? '') : ''}
			/>
			<button type="submit">Search</button>
		</form>
	</div>
</header>

<main class="shell">
	{@render children()}
</main>

<style>
	.visually-hidden {
		position: absolute;
		width: 1px;
		height: 1px;
		overflow: hidden;
		clip-path: inset(50%);
		white-space: nowrap;
	}
</style>
