import { loadRack } from './content';
import type { ContentError, SearchHit } from '$lib/types';

type Scored = SearchHit & { score: number };

function score(haystack: string, needle: string): number {
	const h = haystack.toLowerCase();
	const i = h.indexOf(needle);
	if (i === -1) return 0;
	if (h === needle) return 100;
	if (i === 0) return 60;
	// Word-boundary match beats a match buried mid-word.
	return /\W/.test(h[i - 1] ?? '') ? 40 : 20;
}

function excerpt(text: string, needle: string, span = 140): string {
	const flat = text
		.replace(/\[([^\]]+)\]\([^)]*\)/g, '$1')
		.replace(/[*_`>#]/g, '')
		.replace(/\s+/g, ' ')
		.trim();
	const i = flat.toLowerCase().indexOf(needle);
	if (i === -1) return flat.slice(0, span);
	const start = Math.max(0, i - 40);
	const cut = flat.slice(start, start + span);
	return (start > 0 ? '…' : '') + cut + (start + span < flat.length ? '…' : '');
}

/** Search across device names, recipe names, and troubleshooting symptoms. */
export async function search(
	query: string
): Promise<{ hits: SearchHit[]; errors: ContentError[] }> {
	const needle = query.trim().toLowerCase();
	const { devices, errors } = await loadRack();
	if (!needle) return { hits: [], errors };

	const hits: Scored[] = [];

	for (const entry of devices) {
		const { device } = entry;
		const deviceScore = Math.max(
			score(device.name, needle),
			score(device.id, needle),
			score(device.role, needle) * 0.6,
			...device.tags.map((t) => score(t, needle) * 0.5)
		);
		if (deviceScore > 0) {
			hits.push({
				kind: 'device',
				title: device.name,
				context: device.type,
				snippet: device.role,
				href: `/devices/${device.id}`,
				score: deviceScore
			});
		}

		for (const { recipe } of entry.recipes) {
			const recipeScore = Math.max(
				score(recipe.name, needle),
				score(recipe.id, needle),
				score(recipe.category, needle) * 0.7,
				...recipe.params.map((p) => score(`${p.control} ${p.value}`, needle) * 0.4)
			);
			if (recipeScore > 0) {
				hits.push({
					kind: 'recipe',
					title: recipe.name,
					context: `${device.name} · ${recipe.category}`,
					snippet: recipe.params
						.slice(0, 3)
						.map((p) => `${p.control} ${p.value}`)
						.join(' · '),
					href: `/devices/${device.id}/recipes/${recipe.id}`,
					score: recipeScore
				});
			}
		}

		for (const symptom of entry.symptoms) {
			const symptomScore = Math.max(
				score(symptom.symptom, needle),
				score(symptom.text, needle) * 0.5
			);
			if (symptomScore > 0) {
				hits.push({
					kind: 'symptom',
					title: symptom.symptom,
					context: `${device.name} · troubleshooting`,
					snippet: excerpt(symptom.text, needle),
					href: `/devices/${device.id}#troubleshooting`,
					score: symptomScore
				});
			}
		}
	}

	hits.sort((a, b) => b.score - a.score || a.title.localeCompare(b.title));
	return { hits: hits.map(({ score: _score, ...hit }) => hit), errors };
}
