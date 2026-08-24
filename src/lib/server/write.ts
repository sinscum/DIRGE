import { mkdir, readFile, writeFile } from 'node:fs/promises';
import { join } from 'node:path';
import matter from 'gray-matter';
import { songSchema, type Song } from '$lib/schemas';
import { DATA_DIR } from './content';

/**
 * Writes back to markdown on disk — the only store there is. Nothing is written
 * until it validates, so a bad edit fails loudly instead of corrupting a file.
 */

export class WriteError extends Error {
	issues: { field: string; message: string }[];

	constructor(message: string, issues: { field: string; message: string }[] = []) {
		super(message);
		this.name = 'WriteError';
		this.issues = issues;
	}
}

const SLUG = /^[a-z0-9]+(?:-[a-z0-9]+)*$/;

function songPath(id: string): string {
	if (!SLUG.test(id)) throw new WriteError(`"${id}" is not a usable file name`);
	return join(DATA_DIR, 'songs', `${id}.md`);
}

export function slugify(input: string): string {
	return input
		.toLowerCase()
		.normalize('NFKD')
		.replace(/[^a-z0-9]+/g, '-')
		.replace(/^-|-$/g, '');
}

/** Keep the compact `{ bar: 1, chord: Fm, duration: 4 }` shape the fixtures are written in. */
const YAML_OPTIONS = { flowLevel: 2, lineWidth: 120, noRefs: true } as unknown as Parameters<
	typeof matter.stringify
>[2];

/** js-yaml will not dump `undefined`; an absent optional field should just be absent. */
function prune(value: unknown): unknown {
	if (Array.isArray(value)) return value.map(prune);
	if (value && typeof value === 'object') {
		const out: Record<string, unknown> = {};
		for (const [field, inner] of Object.entries(value as Record<string, unknown>)) {
			if (inner !== undefined) out[field] = prune(inner);
		}
		return out;
	}
	return value;
}

function serialize(body: string, data: Record<string, unknown>): string {
	const content = body.trim() ? `\n${body.trim()}\n` : '\n';
	return matter.stringify(content, prune(data) as Record<string, unknown>, YAML_OPTIONS);
}

function validate(data: unknown): Song {
	const result = songSchema.safeParse(data);
	if (!result.success) {
		throw new WriteError(
			'The song would not pass its own schema, so nothing was written',
			result.error.issues.map((issue) => ({
				field: issue.path.join('.') || '(root)',
				message: issue.message
			}))
		);
	}
	return result.data;
}

/**
 * Merge a patch into an existing song file. Unknown frontmatter keys added by
 * hand are carried through untouched — this app is not the only editor.
 */
export async function saveSong(id: string, patch: Record<string, unknown>): Promise<Song> {
	const path = songPath(id);

	let raw: string;
	try {
		raw = await readFile(path, 'utf8');
	} catch {
		throw new WriteError(`No song file at data/songs/${id}.md`);
	}

	const file = matter(raw);
	const existing = file.data as Record<string, unknown>;
	const merged: Record<string, unknown> = { ...existing, ...patch, id };
	// A null in the patch means "drop this key" — that is how the drone gets turned off.
	for (const [field, value] of Object.entries(patch)) {
		if (value === null) delete merged[field];
	}
	const song = validate(merged);

	const out: Record<string, unknown> = { ...merged, ...song };
	for (const [field, value] of Object.entries(patch)) {
		if (value === null) delete out[field];
	}
	await writeFile(path, serialize(file.content, out), 'utf8');
	return song;
}

export type NewSong = {
	title: string;
	key: string;
	mode: string;
	bpm: number;
};

export async function createSong(input: NewSong): Promise<Song> {
	const id = slugify(input.title);
	if (!id) throw new WriteError('Give the song a title with at least one letter or number');

	const path = songPath(id);
	try {
		await readFile(path, 'utf8');
		throw new WriteError(`data/songs/${id}.md already exists`);
	} catch (err) {
		if (err instanceof WriteError) throw err;
	}

	const song = validate({
		id,
		title: input.title.trim(),
		key: input.key,
		mode: input.mode,
		bpm: input.bpm,
		status: 'sketch',
		progression: [],
		layers: [],
		suno_overrides: { reference_artists: [], production: '' }
	});

	await mkdir(join(DATA_DIR, 'songs'), { recursive: true });
	await writeFile(path, serialize('', song), 'utf8');
	return song;
}
