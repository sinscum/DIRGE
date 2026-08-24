import { error, fail } from '@sveltejs/kit';
import { z } from 'zod';
import { loadSongs } from '$lib/server/content';
import { saveSong, WriteError } from '$lib/server/write';
import { droneSchema } from '$lib/schemas';
import { MODES, KEYS, chordFromSymbol, renumber } from '$lib/music';
import type { Actions, PageServerLoad } from './$types';

export const load: PageServerLoad = async ({ params }) => {
	const { songs, errors } = await loadSongs();
	const entry = songs.find((s) => s.song.id === params.song);
	if (!entry) {
		error(404, `No song with id "${params.song}" under data/songs`);
	}

	return { song: entry.song, file: entry.file, errors };
};

/** What the builder is allowed to write. Layers and Suno overrides belong to later phases. */
const payloadSchema = z.object({
	key: z.string().refine((k) => KEYS.includes(k), 'unknown key'),
	mode: z.string().refine((m) => MODES.some((mode) => mode.id === m), 'unknown mode'),
	bpm: z.number().positive(),
	progression: z
		.array(
			z.object({
				chord: z.string().min(1),
				duration: z.number().int().positive().max(64)
			})
		)
		.max(128),
	drone: droneSchema.nullable()
});

export const actions: Actions = {
	save: async ({ params, request }) => {
		const form = await request.formData();

		let parsed: unknown;
		try {
			parsed = JSON.parse(String(form.get('payload') ?? ''));
		} catch {
			return fail(400, {
				message: 'The editor sent something that was not valid JSON',
				issues: []
			});
		}

		const result = payloadSchema.safeParse(parsed);
		if (!result.success) {
			return fail(400, {
				message: 'The progression did not validate, so nothing was written',
				issues: result.error.issues.map((issue) => ({
					field: issue.path.join('.') || '(root)',
					message: issue.message
				}))
			});
		}

		const { key, mode, bpm, progression, drone } = result.data;

		const unreadable = progression.find((step) => !chordFromSymbol(step.chord));
		if (unreadable) {
			return fail(400, {
				message: `"${unreadable.chord}" is not a chord this app can read back, so nothing was written`,
				issues: []
			});
		}

		try {
			await saveSong(params.song, {
				key,
				mode,
				bpm,
				progression: renumber(progression),
				drone: drone && drone.note ? drone : null
			});
		} catch (err) {
			if (err instanceof WriteError) {
				return fail(400, { message: err.message, issues: err.issues });
			}
			throw err;
		}

		return { saved: true };
	}
};
