<script lang="ts">
	import type { ContentError } from '$lib/types';

	let { errors }: { errors: ContentError[] } = $props();
</script>

{#if errors.length}
	<section class="errors" aria-label="Content validation errors">
		<h2>{errors.length} file{errors.length === 1 ? '' : 's'} failed validation</h2>
		<p class="dim">Everything else on this page loaded. Fix the file and refresh.</p>
		{#each errors as error (error.file)}
			<div class="error">
				<code>{error.file}</code>
				<ul>
					{#each error.issues as issue (issue.field + issue.message)}
						<li><span class="field">{issue.field}</span> — {issue.message}</li>
					{/each}
				</ul>
			</div>
		{/each}
	</section>
{/if}

<style>
	.errors {
		border: 1px solid var(--oxblood-bright);
		border-left-width: 4px;
		background: color-mix(in srgb, var(--oxblood) 18%, var(--bg));
		padding: 0.9rem 1.1rem;
		margin-bottom: 1.5rem;
		border-radius: var(--radius);
	}

	.errors h2 {
		color: var(--oxblood-bright);
		font-size: 1rem;
		margin-bottom: 0.25rem;
	}

	.errors p {
		font-size: 0.85rem;
		margin-bottom: 0.75rem;
	}

	.error + .error {
		margin-top: 0.6rem;
	}

	.error code {
		color: var(--oxblood-bright);
		border-color: var(--oxblood);
	}

	ul {
		margin: 0.3rem 0 0;
		padding-left: 1.1rem;
		font-size: 0.85rem;
	}

	.field {
		color: var(--oxblood-bright);
	}
</style>
