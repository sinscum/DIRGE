import { marked } from 'marked';
import { DEVICE_SECTIONS, sectionSlug, type RenderedSection, type Symptom } from '$lib/sections';

marked.setOptions({ gfm: true, breaks: false });

export function render(md: string): string {
	return marked.parse(md.trim(), { async: false });
}

/** Split a markdown body on its `##` headings. Text above the first one is dropped on the floor. */
export function splitSections(body: string): Map<string, string> {
	const out = new Map<string, string>();
	const lines = body.split(/\r?\n/);
	let current: string | null = null;
	let buffer: string[] = [];

	const flush = () => {
		if (current !== null) out.set(current.toLowerCase(), buffer.join('\n').trim());
		buffer = [];
	};

	for (const line of lines) {
		const heading = /^##\s+(.+?)\s*$/.exec(line);
		if (heading && !line.startsWith('###')) {
			flush();
			current = heading[1];
		} else if (current !== null) {
			buffer.push(line);
		}
	}
	flush();
	return out;
}

/** The five fixed device sections, in order, with a stub where one is missing. */
export function deviceSections(body: string): RenderedSection[] {
	const found = splitSections(body);
	return DEVICE_SECTIONS.map(({ title, hint }) => {
		const raw = found.get(title.toLowerCase());
		return {
			title,
			slug: sectionSlug(title),
			hint,
			missing: !raw,
			html: raw ? render(raw) : ''
		};
	});
}

/** Symptoms are the `###` headings inside Troubleshooting — these feed search. */
export function parseSymptoms(body: string): Symptom[] {
	const raw = splitSections(body).get('troubleshooting');
	if (!raw) return [];

	const out: Symptom[] = [];
	let current: Symptom | null = null;
	for (const line of raw.split(/\r?\n/)) {
		const heading = /^###\s+(.+?)\s*$/.exec(line);
		if (heading) {
			if (current) out.push(current);
			current = { symptom: heading[1], text: '' };
		} else if (current) {
			current.text = `${current.text}\n${line}`.trim();
		}
	}
	if (current) out.push(current);
	return out;
}
