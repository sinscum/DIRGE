import { readdir, readFile, stat } from 'node:fs/promises';
import { join, relative, resolve } from 'node:path';
import matter from 'gray-matter';
import type { ZodType } from 'zod';
import {
	deviceSchema,
	recipeSchema,
	songSchema,
	type Device,
	type Recipe,
	type Song
} from '$lib/schemas';
import { deviceSections, parseSymptoms, render } from './markdown';
import type { RenderedSection, Symptom } from '$lib/sections';
import type { ContentError } from '$lib/types';

/** The filesystem is the database. Everything is read on request so an editor save shows up on refresh. */
export const DATA_DIR = resolve(process.env.DIRGE_DATA_DIR ?? 'data');

export type { ContentError };

export type DeviceEntry = {
	file: string;
	device: Device;
	sections: RenderedSection[];
	symptoms: Symptom[];
	recipes: RecipeEntry[];
};

export type RecipeEntry = {
	file: string;
	recipe: Recipe;
	html: string;
};

export type SongEntry = {
	file: string;
	song: Song;
};

export type Rack = {
	devices: DeviceEntry[];
	errors: ContentError[];
};

function rel(path: string): string {
	return relative(DATA_DIR, path) || path;
}

async function exists(path: string): Promise<boolean> {
	try {
		await stat(path);
		return true;
	} catch {
		return false;
	}
}

async function listMarkdown(dir: string): Promise<string[]> {
	if (!(await exists(dir))) return [];
	const entries = await readdir(dir, { withFileTypes: true });
	return entries
		.filter((e) => e.isFile() && e.name.endsWith('.md'))
		.map((e) => join(dir, e.name))
		.sort();
}

async function listDirectories(dir: string): Promise<string[]> {
	if (!(await exists(dir))) return [];
	const entries = await readdir(dir, { withFileTypes: true });
	return entries
		.filter((e) => e.isDirectory() && !e.name.startsWith('.'))
		.map((e) => join(dir, e.name))
		.sort();
}

type Parsed<T> = { value: T; body: string };

/** Parse one file. A schema failure comes back as an error record, never as a throw. */
async function parseFile<T>(
	path: string,
	schema: ZodType<T>,
	errors: ContentError[]
): Promise<Parsed<T> | null> {
	let raw: string;
	try {
		raw = await readFile(path, 'utf8');
	} catch (err) {
		errors.push({ file: rel(path), issues: [{ field: '', message: String(err) }] });
		return null;
	}

	let data: unknown;
	let body: string;
	try {
		const file = matter(raw);
		data = file.data;
		body = file.content;
	} catch (err) {
		const message = err instanceof Error ? err.message : String(err);
		errors.push({ file: rel(path), issues: [{ field: 'frontmatter', message }] });
		return null;
	}

	const result = schema.safeParse(data);
	if (!result.success) {
		errors.push({
			file: rel(path),
			issues: result.error.issues.map((issue) => ({
				field: issue.path.join('.') || '(root)',
				message: issue.message
			}))
		});
		return null;
	}

	return { value: result.data, body };
}

async function loadRecipes(deviceDir: string, errors: ContentError[]): Promise<RecipeEntry[]> {
	const files = await listMarkdown(join(deviceDir, 'recipes'));
	const out: RecipeEntry[] = [];
	for (const file of files) {
		const parsed = await parseFile(file, recipeSchema, errors);
		if (!parsed) continue;
		out.push({
			file: rel(file),
			recipe: parsed.value,
			html: parsed.body.trim() ? render(parsed.body) : ''
		});
	}
	return out.sort((a, b) => a.recipe.name.localeCompare(b.recipe.name));
}

/** Every device with its recipes, plus whatever failed to validate along the way. */
export async function loadRack(): Promise<Rack> {
	const errors: ContentError[] = [];
	const devices: DeviceEntry[] = [];

	for (const dir of await listDirectories(join(DATA_DIR, 'devices'))) {
		const file = join(dir, 'device.md');
		if (!(await exists(file))) {
			errors.push({
				file: rel(file),
				issues: [{ field: '', message: 'device folder has no device.md' }]
			});
			continue;
		}
		const parsed = await parseFile(file, deviceSchema, errors);
		if (!parsed) continue;
		devices.push({
			file: rel(file),
			device: parsed.value,
			sections: deviceSections(parsed.body),
			symptoms: parseSymptoms(parsed.body),
			recipes: await loadRecipes(dir, errors)
		});
	}

	devices.sort((a, b) => a.device.name.localeCompare(b.device.name));
	return { devices, errors };
}

export async function loadDevice(
	id: string
): Promise<{ entry: DeviceEntry | null; errors: ContentError[] }> {
	const rack = await loadRack();
	return { entry: rack.devices.find((d) => d.device.id === id) ?? null, errors: rack.errors };
}

export async function loadSongs(): Promise<{ songs: SongEntry[]; errors: ContentError[] }> {
	const errors: ContentError[] = [];
	const songs: SongEntry[] = [];
	for (const file of await listMarkdown(join(DATA_DIR, 'songs'))) {
		const parsed = await parseFile(file, songSchema, errors);
		if (!parsed) continue;
		songs.push({ file: rel(file), song: parsed.value });
	}
	songs.sort((a, b) => a.song.title.localeCompare(b.song.title));
	return { songs, errors };
}

export type UsedIn = {
	song: Song;
	slots: string[];
};

/** Back-reference: which songs point a layer at this recipe id. */
export async function usedIn(recipeId: string): Promise<{ uses: UsedIn[]; errors: ContentError[] }> {
	const { songs, errors } = await loadSongs();
	const uses: UsedIn[] = [];
	for (const { song } of songs) {
		const slots = song.layers.filter((l) => l.recipe === recipeId).map((l) => l.slot);
		if (slots.length) uses.push({ song, slots });
	}
	return { uses, errors };
}
