import { error } from '@sveltejs/kit';
import { loadDevice } from '$lib/server/content';
import type { PageServerLoad } from './$types';

export const load: PageServerLoad = async ({ params }) => {
	const { entry, errors } = await loadDevice(params.device);
	if (!entry) {
		error(404, `No device with id "${params.device}" under data/devices`);
	}

	return {
		device: entry.device,
		file: entry.file,
		sections: entry.sections,
		recipes: entry.recipes.map(({ recipe }) => ({
			id: recipe.id,
			name: recipe.name,
			category: recipe.category,
			key_agnostic: recipe.key_agnostic,
			paramCount: recipe.params.length,
			summary: recipe.params
				.slice(0, 3)
				.map((p) => `${p.control} ${p.value}`)
				.join(' · ')
		})),
		errors
	};
};
