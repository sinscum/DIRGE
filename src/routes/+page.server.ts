import { loadRack } from '$lib/server/content';
import { deviceTypes, type DeviceType } from '$lib/schemas';
import type { PageServerLoad } from './$types';

export const load: PageServerLoad = async () => {
	const { devices, errors } = await loadRack();

	const cards = devices.map(({ device, recipes }) => ({
		id: device.id,
		name: device.name,
		type: device.type,
		role: device.role,
		tags: device.tags,
		midi_channel: device.midi_channel,
		recipeCount: recipes.length
	}));

	const groups = deviceTypes
		.map((type: DeviceType) => ({ type, devices: cards.filter((c) => c.type === type) }))
		.filter((group) => group.devices.length > 0);

	return { groups, errors, deviceCount: cards.length };
};
