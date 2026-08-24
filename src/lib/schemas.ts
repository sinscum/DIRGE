import { z } from 'zod';

/**
 * Schemas for everything under /data. These run on read and on write — a file
 * that fails validation is surfaced in the UI, never dropped and never crashed on.
 */

/** YAML happily gives us numbers where we want text (firmware: 6.0, value: 62). */
const text = z.union([z.string(), z.number(), z.boolean()]).transform((v) => String(v));

const slug = z
	.string()
	.min(1)
	.regex(/^[a-z0-9]+(?:-[a-z0-9]+)*$/, 'must be lowercase words separated by hyphens');

export const deviceTypes = ['synth', 'drum', 'fx', 'controller', 'recorder', 'utility'] as const;
export const recipeCategories = ['bass', 'pad', 'lead', 'arp', 'perc', 'texture', 'fx'] as const;
export const songStatuses = ['sketch', 'arranged', 'tracked', 'mixed', 'done'] as const;

export type DeviceType = (typeof deviceTypes)[number];
export type RecipeCategory = (typeof recipeCategories)[number];
export type SongStatus = (typeof songStatuses)[number];

export const deviceSchema = z.object({
	id: slug,
	name: z.string().min(1),
	type: z.enum(deviceTypes),
	firmware: text.optional(),
	polyphony: z.number().int().positive().optional(),
	engines: z.array(text).default([]),
	io: z.record(z.string(), text).default({}),
	midi_channel: z.number().int().min(1).max(16).optional(),
	role: z.string().default(''),
	manual_url: z.string().default(''),
	tags: z.array(text).default([])
});

export const recipeParamSchema = z.object({
	section: z.string().min(1),
	control: z.string().min(1),
	value: text
});

export const recipeSchema = z.object({
	id: slug,
	name: z.string().min(1),
	device: slug,
	category: z.enum(recipeCategories),
	key_agnostic: z.boolean().default(false),
	params: z.array(recipeParamSchema).default([]),
	depends_on: z
		.object({
			clock: text.optional(),
			fx_chain: z.array(text).default([])
		})
		.default({ fx_chain: [] }),
	audio_ref: z.string().default('')
});

export const progressionStepSchema = z.object({
	bar: z.number().int().positive(),
	chord: z.string().min(1),
	duration: z.number().positive()
});

export const layerSchema = z.object({
	slot: z.string().min(1),
	device: slug.optional(),
	recipe: slug.optional(),
	register: z.string().default(''),
	enters_bar: z.number().int().positive(),
	exits_bar: z.number().int().positive(),
	notes: z.string().default('')
});

export const songSchema = z.object({
	id: slug,
	title: z.string().min(1),
	key: z.string().min(1),
	mode: z.string().min(1),
	bpm: z.number().positive(),
	status: z.enum(songStatuses),
	progression: z.array(progressionStepSchema).default([]),
	layers: z.array(layerSchema).default([]),
	suno_overrides: z
		.object({
			reference_artists: z.array(text).default([]),
			production: z.string().default('')
		})
		.default({ reference_artists: [], production: '' })
});

export type Device = z.output<typeof deviceSchema>;
export type Recipe = z.output<typeof recipeSchema>;
export type RecipeParam = z.output<typeof recipeParamSchema>;
export type Song = z.output<typeof songSchema>;
export type Layer = z.output<typeof layerSchema>;
