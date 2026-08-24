/** Shared shapes that cross the server/client line. */

export type ContentError = {
	/** Path relative to /data, so the message tells you which file to open. */
	file: string;
	issues: { field: string; message: string }[];
};

export type SearchHit = {
	kind: 'device' | 'recipe' | 'symptom';
	title: string;
	context: string;
	snippet: string;
	href: string;
};
