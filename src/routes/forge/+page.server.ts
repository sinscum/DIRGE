import { fail, redirect } from '@sveltejs/kit';
import { loadSongs } from '$lib/server/content';
import { createSong, WriteError } from '$lib/server/write';
import { MODES, KEYS, totalBars } from '$lib/music';
import type { Actions, PageServerLoad } from './$types';

export const load: PageServerLoad = async () => {
	const { songs, errors } = await loadSongs();
	return {
		songs: songs.map(({ song, file }) => ({
			id: song.id,
			title: song.title,
			key: song.key,
			mode: song.mode,
			bpm: song.bpm,
			status: song.status,
			bars: totalBars(song.progression),
			chords: song.progression.length,
			layers: song.layers.length,
			file
		})),
		errors
	};
};

export const actions: Actions = {
	create: async ({ request }) => {
		const form = await request.formData();
		const title = String(form.get('title') ?? '').trim();
		const key = String(form.get('key') ?? 'F');
		const mode = String(form.get('mode') ?? 'phrygian');
		const bpm = Number(form.get('bpm') ?? 96);

		if (!title) return fail(400, { message: 'Give the song a title' });
		if (!KEYS.includes(key)) return fail(400, { message: `"${key}" is not a key` });
		if (!MODES.some((m) => m.id === mode)) return fail(400, { message: `"${mode}" is not a mode` });
		if (!Number.isFinite(bpm) || bpm <= 0) return fail(400, { message: 'BPM must be a number' });

		let id: string;
		try {
			const song = await createSong({ title, key, mode, bpm });
			id = song.id;
		} catch (err) {
			if (err instanceof WriteError) {
				return fail(400, { message: err.message, issues: err.issues });
			}
			throw err;
		}

		redirect(303, `/forge/${id}`);
	}
};
