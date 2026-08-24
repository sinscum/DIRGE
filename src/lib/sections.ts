/** The fixed section order every device.md is expected to carry. */
export const DEVICE_SECTIONS = [
	{
		title: 'Vitals',
		hint: 'Engines, voices, tuning quirks — the facts you forget between sessions.'
	},
	{
		title: 'Shortcuts',
		hint: 'Key combos, hidden menus, factory reset.'
	},
	{
		title: 'Signal chain',
		hint: 'Where this box sits in the rig — ins, outs, clock, what it feeds.'
	},
	{
		title: 'Troubleshooting',
		hint: 'One `###` heading per symptom, then cause, then fix.'
	},
	{
		title: 'Resources',
		hint: 'Manual, firmware downloads, the one forum thread that solved it.'
	}
] as const;

export type SectionTitle = (typeof DEVICE_SECTIONS)[number]['title'];

export type RenderedSection = {
	title: string;
	slug: string;
	html: string;
	hint: string;
	missing: boolean;
};

/** A parsed `### symptom` block from the Troubleshooting section. */
export type Symptom = {
	symptom: string;
	text: string;
};

export function sectionSlug(title: string): string {
	return title
		.toLowerCase()
		.replace(/[^a-z0-9]+/g, '-')
		.replace(/^-|-$/g, '');
}
