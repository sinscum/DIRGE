import { search } from '$lib/server/search';
import type { PageServerLoad } from './$types';

export const load: PageServerLoad = async ({ url }) => {
	const query = url.searchParams.get('q') ?? '';
	const { hits, errors } = await search(query);
	return { query, hits, errors };
};
