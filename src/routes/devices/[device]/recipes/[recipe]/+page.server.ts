import { error } from '@sveltejs/kit';
import { loadDevice, usedIn } from '$lib/server/content';
import type { RecipeParam } from '$lib/schemas';
import type { PageServerLoad } from './$types';

/** Group the flat param list by `section`, keeping the order the file wrote them in. */
function groupParams(params: RecipeParam[]): { section: string; params: RecipeParam[] }[] {
	const groups: { section: string; params: RecipeParam[] }[] = [];
	for (const param of params) {
		const group = groups.find((g) => g.section === param.section);
		if (group) group.params.push(param);
		else groups.push({ section: param.section, params: [param] });
	}
	return groups;
}

export const load: PageServerLoad = async ({ params }) => {
	const { entry, errors } = await loadDevice(params.device);
	if (!entry) {
		error(404, `No device with id "${params.device}" under data/devices`);
	}

	const match = entry.recipes.find((r) => r.recipe.id === params.recipe);
	if (!match) {
		error(404, `No recipe with id "${params.recipe}" on ${entry.device.name}`);
	}

	const { uses, errors: songErrors } = await usedIn(match.recipe.id);

	return {
		device: { id: entry.device.id, name: entry.device.name, type: entry.device.type },
		recipe: match.recipe,
		file: match.file,
		html: match.html,
		groups: groupParams(match.recipe.params),
		uses: uses.map(({ song, slots }) => ({
			id: song.id,
			title: song.title,
			key: song.key,
			mode: song.mode,
			bpm: song.bpm,
			status: song.status,
			slots
		})),
		errors: [...errors, ...songErrors]
	};
};
