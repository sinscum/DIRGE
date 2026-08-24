/**
 * Note spelling, modes, and triads. Letter-based so a mode in F spells Gb, not F#.
 * Pure and shared — the builder uses it in the browser, the writer uses it on the server.
 */

export type Note = {
	/** 0 = C … 6 = B */
	letter: number;
	/** semitone offset: -2 … 2 */
	acc: number;
};

export type Quality =
	| 'major'
	| 'minor'
	| 'dim'
	| 'aug'
	| 'sus2'
	| 'sus4'
	| 'majb5'
	| 'minsharp5'
	| 'alt';

export type Chord = {
	symbol: string;
	root: Note;
	quality: Quality;
	notes: Note[];
	/** Roman numeral in the current mode, when the chord came from the palette. */
	numeral?: string;
	/** Why the chord is being offered: a passing colour, a mediant move, the Phrygian bII. */
	tag?: 'passing' | 'mediant' | 'flat-two';
	/** For mediants: the move away from the chord it was suggested from. */
	move?: string;
};

const LETTER_SEMITONES = [0, 2, 4, 5, 7, 9, 11];
const LETTER_NAMES = ['C', 'D', 'E', 'F', 'G', 'A', 'B'];

export const MODES = [
	{ id: 'aeolian', name: 'Aeolian', primary: true, intervals: [0, 2, 3, 5, 7, 8, 10] },
	{ id: 'phrygian', name: 'Phrygian', primary: true, intervals: [0, 1, 3, 5, 7, 8, 10] },
	{ id: 'harmonic-minor', name: 'Harmonic minor', primary: true, intervals: [0, 2, 3, 5, 7, 8, 11] },
	{ id: 'locrian', name: 'Locrian', primary: true, intervals: [0, 1, 3, 5, 6, 8, 10] },
	{ id: 'dorian', name: 'Dorian', primary: true, intervals: [0, 2, 3, 5, 7, 9, 10] },
	{
		id: 'phrygian-dominant',
		name: 'Phrygian dominant',
		primary: false,
		intervals: [0, 1, 4, 5, 7, 8, 10]
	},
	{ id: 'hungarian-minor', name: 'Hungarian minor', primary: false, intervals: [0, 2, 3, 6, 7, 8, 11] },
	{
		id: 'double-harmonic',
		name: 'Double harmonic',
		primary: false,
		intervals: [0, 1, 4, 5, 7, 8, 11]
	},
	{ id: 'melodic-minor', name: 'Melodic minor', primary: false, intervals: [0, 2, 3, 5, 7, 9, 11] },
	{ id: 'ionian', name: 'Ionian', primary: false, intervals: [0, 2, 4, 5, 7, 9, 11] },
	{ id: 'lydian', name: 'Lydian', primary: false, intervals: [0, 2, 4, 6, 7, 9, 11] },
	{ id: 'mixolydian', name: 'Mixolydian', primary: false, intervals: [0, 2, 4, 5, 7, 9, 10] }
] as const;

export type ModeId = (typeof MODES)[number]['id'];

export const KEYS = [
	'C',
	'C#',
	'Db',
	'D',
	'D#',
	'Eb',
	'E',
	'F',
	'F#',
	'Gb',
	'G',
	'G#',
	'Ab',
	'A',
	'A#',
	'Bb',
	'B'
];

/** Tempo bands, in the language they get talked about in. */
export const TEMPO_BANDS = [
	{ label: 'stalking dread', low: 80, high: 110, pick: 96 },
	{ label: 'Carpenter pulse', low: 120, high: 140, pick: 130 },
	{ label: 'chase', low: 140, high: 160, pick: 150 }
];

export function findMode(id: string) {
	return MODES.find((m) => m.id === id);
}

export function noteName(note: Note): string {
	const acc = note.acc === 0 ? '' : note.acc > 0 ? '#'.repeat(note.acc) : 'b'.repeat(-note.acc);
	return LETTER_NAMES[note.letter] + acc;
}

/** Screen spelling — flats and sharps as real glyphs, for headings and buttons. */
export function prettyNote(note: Note): string {
	return noteName(note).replace(/#/g, '♯').replace(/b/g, '♭');
}

export function parseNote(name: string): Note | null {
	const match = /^([A-Ga-g])([#b♯♭]{0,2})$/.exec(name.trim());
	if (!match) return null;
	const letter = LETTER_NAMES.indexOf(match[1].toUpperCase());
	const marks = match[2].replace(/♯/g, '#').replace(/♭/g, 'b');
	const acc = marks.startsWith('b') ? -marks.length : marks.length ? marks.length : 0;
	return { letter, acc };
}

export function pitchClass(note: Note): number {
	return (((LETTER_SEMITONES[note.letter] + note.acc) % 12) + 12) % 12;
}

/** Move by a letter distance and a semitone distance at once — this is what keeps spelling honest. */
export function noteAt(root: Note, letterSteps: number, semitones: number): Note {
	const raw = root.letter + letterSteps;
	const letter = ((raw % 7) + 7) % 7;
	const octaves = Math.floor(raw / 7);
	const target = LETTER_SEMITONES[root.letter] + root.acc + semitones;
	const natural = LETTER_SEMITONES[letter] + 12 * octaves;
	return { letter, acc: target - natural };
}

export function scaleNotes(tonic: Note, modeId: string): Note[] {
	const mode = findMode(modeId) ?? MODES[0];
	return mode.intervals.map((semitones, degree) => noteAt(tonic, degree, semitones));
}

const QUALITY_INTERVALS: Record<Exclude<Quality, 'alt'>, [number, number]> = {
	major: [4, 7],
	minor: [3, 7],
	dim: [3, 6],
	aug: [4, 8],
	sus2: [2, 7],
	sus4: [5, 7],
	// The exotic modes throw these two; parenthesised so "Gb(b5)" can't be read as a double flat.
	majb5: [4, 6],
	minsharp5: [3, 8]
};

const QUALITY_SUFFIX: Record<Quality, string> = {
	major: '',
	minor: 'm',
	dim: 'dim',
	aug: 'aug',
	sus2: 'sus2',
	sus4: 'sus4',
	majb5: '(b5)',
	minsharp5: 'm(#5)',
	alt: ''
};

const QUALITY_LABEL: Record<Quality, string> = {
	major: 'major',
	minor: 'minor',
	dim: 'diminished',
	aug: 'augmented',
	sus2: 'sus2',
	sus4: 'sus4',
	majb5: 'major ♭5',
	minsharp5: 'minor ♯5',
	alt: 'no plain triad name'
};

export function qualityLabel(quality: Quality): string {
	return QUALITY_LABEL[quality];
}

function qualityFrom(third: number, fifth: number): Quality {
	for (const [quality, [a, b]] of Object.entries(QUALITY_INTERVALS)) {
		if (a === third && b === fifth) return quality as Quality;
	}
	return 'alt';
}

export function triad(root: Note, quality: Quality): Note[] {
	const [third, fifth] = QUALITY_INTERVALS[quality === 'alt' ? 'major' : quality];
	return [root, noteAt(root, 2, third), noteAt(root, 4, fifth)];
}

export function chordSymbol(root: Note, quality: Quality): string {
	return noteName(root) + QUALITY_SUFFIX[quality];
}

const NUMERALS = ['I', 'II', 'III', 'IV', 'V', 'VI', 'VII'];
const MAJOR = [0, 2, 4, 5, 7, 9, 11];

function numeralFor(degree: number, scaleNote: Note, tonic: Note, quality: Quality): string {
	const offset = (((pitchClass(scaleNote) - pitchClass(tonic) - MAJOR[degree]) % 12) + 12) % 12;
	const prefix = offset === 0 ? '' : offset === 11 ? '♭' : offset === 1 ? '♯' : '';
	const base = NUMERALS[degree];
	const body = quality === 'minor' || quality === 'dim' ? base.toLowerCase() : base;
	const suffix = quality === 'dim' ? '°' : quality === 'aug' ? '+' : '';
	return prefix + body + suffix;
}

/** The seven diatonic triads of a mode, in order, tagged where they want a warning label. */
export function diatonicTriads(tonic: Note, modeId: string): Chord[] {
	const scale = scaleNotes(tonic, modeId);
	return scale.map((root, degree) => {
		const third = scale[(degree + 2) % 7];
		const fifth = scale[(degree + 4) % 7];
		const thirdGap = (((pitchClass(third) - pitchClass(root)) % 12) + 12) % 12;
		const fifthGap = (((pitchClass(fifth) - pitchClass(root)) % 12) + 12) % 12;
		const quality = qualityFrom(thirdGap, fifthGap);
		const notes = quality === 'alt' ? [root, third, fifth] : triad(root, quality);
		const numeral = numeralFor(degree, root, tonic, quality);
		const isFlatTwo = degree === 1 && numeral.startsWith('♭');
		return {
			// An unnameable stack gets no symbol — the palette shows it, but you can't place it.
			symbol: quality === 'alt' ? '' : chordSymbol(root, quality),
			root,
			quality,
			notes,
			numeral,
			tag: isFlatTwo ? 'flat-two' : quality === 'dim' || quality === 'aug' ? 'passing' : undefined
		} satisfies Chord;
	});
}

/**
 * Chromatic mediants: major triads a third away from where you are.
 * ♭VI, ♭III and III — the moves that are wrong in a good way.
 */
export function chromaticMediants(from: Note): Chord[] {
	const moves: { move: string; letterSteps: number; semitones: number }[] = [
		{ move: '♭VI', letterSteps: 5, semitones: 8 },
		{ move: '♭III', letterSteps: 2, semitones: 3 },
		{ move: 'III', letterSteps: 2, semitones: 4 }
	];
	return moves.map(({ move, letterSteps, semitones }) => {
		const root = simplify(noteAt(from, letterSteps, semitones));
		return {
			symbol: chordSymbol(root, 'major'),
			root,
			quality: 'major' as Quality,
			notes: triad(root, 'major'),
			tag: 'mediant' as const,
			move
		};
	});
}

const FLAT_NAMES = ['C', 'Db', 'D', 'Eb', 'E', 'F', 'Gb', 'G', 'Ab', 'A', 'Bb', 'B'];
const SHARP_NAMES = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'];

/**
 * Strict spelling is right inside a scale and wrong on a chromatic move — nobody
 * wants an F♭ major chord. Respell anything with a double accidental, or Fb/Cb/E#/B#.
 */
export function simplify(note: Note): Note {
	const awkward =
		Math.abs(note.acc) > 1 ||
		(note.acc === -1 && (note.letter === 0 || note.letter === 3)) ||
		(note.acc === 1 && (note.letter === 2 || note.letter === 6));
	if (!awkward) return note;
	const table = note.acc < 0 ? FLAT_NAMES : SHARP_NAMES;
	return parseNote(table[pitchClass(note)]) ?? note;
}

/** The tritone above a note — the pedal that refuses to resolve. */
export function tritoneAbove(note: Note): Note {
	return simplify(noteAt(note, 3, 6));
}

const SUFFIXES: { suffix: string; quality: Quality }[] = [
	{ suffix: 'sus2', quality: 'sus2' },
	{ suffix: 'sus4', quality: 'sus4' },
	{ suffix: '(b5)', quality: 'majb5' },
	{ suffix: 'm(#5)', quality: 'minsharp5' },
	{ suffix: 'dim', quality: 'dim' },
	{ suffix: '°', quality: 'dim' },
	{ suffix: 'aug', quality: 'aug' },
	{ suffix: '+', quality: 'aug' },
	{ suffix: 'min', quality: 'minor' },
	{ suffix: 'm', quality: 'minor' },
	{ suffix: 'maj', quality: 'major' },
	{ suffix: '', quality: 'major' }
];

/** Read a chord back out of a song file. */
export function chordFromSymbol(symbol: string): Chord | null {
	const match = /^([A-Ga-g][#b♯♭]{0,2})(.*)$/.exec(symbol.trim());
	if (!match) return null;
	const root = parseNote(match[1]);
	if (!root) return null;
	const rest = match[2].trim();
	const found = SUFFIXES.find((s) => s.suffix.toLowerCase() === rest.toLowerCase());
	if (!found) return null;
	return {
		symbol: chordSymbol(root, found.quality),
		root,
		quality: found.quality,
		notes: triad(root, found.quality)
	};
}

export type ProgressionStep = { bar: number; chord: string; duration: number };

/** Bars are derived from durations — the timeline is the truth, `bar` is bookkeeping. */
export function renumber(steps: { chord: string; duration: number }[]): ProgressionStep[] {
	let bar = 1;
	return steps.map((step) => {
		const out = { bar, chord: step.chord, duration: step.duration };
		bar += step.duration;
		return out;
	});
}

export function totalBars(steps: { duration: number }[]): number {
	return steps.reduce((sum, step) => sum + step.duration, 0);
}
