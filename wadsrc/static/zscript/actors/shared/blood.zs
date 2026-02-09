/*
** blood.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

// Blood sprite ------------------------------------------------------------

class Blood : Actor
{
	const BLOOD_POOL_DAMAGE_THRESHOLD = 20;

	action void A_BloodLandTick()
	{
		if (self.special1 != 0 || self.pos.Z > self.floorz + 1)
		{
			return;
		}
		self.special1 = 1;

		if (self.GetFloorTerrain().IsLiquid)
		{
			return;
		}

		if (self.health >= BLOOD_POOL_DAMAGE_THRESHOLD)
		{
			let pool = self.Spawn("BloodPool", (self.pos.xy, self.floorz), ALLOW_REPLACE);
			if (pool)
			{
				pool.Translation = self.Translation;
			}
		}
		else
		{
			let pool = self.Spawn("SmallBloodPool", (self.pos.xy, self.floorz), ALLOW_REPLACE);
			if (pool)
			{
				pool.Translation = self.Translation;
			}
		}
	}

	Default
	{
		Mass 5;
		+NOBLOCKMAP
		+NOTELEPORT
		+ALLOWPARTICLES
	}
	States
	{
	Spawn:
		BLUD C 8 A_BloodLandTick;
		BLUD B 8 A_BloodLandTick;
		BLUD A 8 A_BloodLandTick;
		Stop;
	Spray:
		SPRY ABCDEF 3;
		SPRY G 2;
		Stop;
	}
}

// Blood splatter -----------------------------------------------------------

class BloodSplatter : Actor
{
	const BLOOD_POOL_DAMAGE_THRESHOLD = 20;

	action void A_BloodLandTick()
	{
		if (self.special1 != 0 || self.pos.Z > self.floorz + 1)
		{
			return;
		}
		self.special1 = 1;

		if (self.GetFloorTerrain().IsLiquid)
		{
			return;
		}

		if (self.health >= BLOOD_POOL_DAMAGE_THRESHOLD)
		{
			let pool = self.Spawn("BloodPool", (self.pos.xy, self.floorz), ALLOW_REPLACE);
			if (pool)
			{
				pool.Translation = self.Translation;
			}
		}
		else
		{
			let pool = self.Spawn("SmallBloodPool", (self.pos.xy, self.floorz), ALLOW_REPLACE);
			if (pool)
			{
				pool.Translation = self.Translation;
			}
		}
	}

	Default
	{
		Radius 2;
		Height 4;
		+NOBLOCKMAP
		+MISSILE
		+DROPOFF
		+NOTELEPORT
		+CANNOTPUSH
		+ALLOWPARTICLES
		Mass 5;
	}
	States
	{
	Spawn:
		BLUD C 8 A_BloodLandTick;
		BLUD B 8 A_BloodLandTick;
		BLUD A 8 A_BloodLandTick;
		Stop;
	Death:
		BLUD A 6 A_BloodLandTick;
		Stop;
	}
}
	
// Axe Blood ----------------------------------------------------------------

class AxeBlood : Actor
{
	Default
	{
		Radius 2;
		Height 4;
		+NOBLOCKMAP
		+NOGRAVITY
		+DROPOFF
		+NOTELEPORT
		+CANNOTPUSH
		+ALLOWPARTICLES
		Mass 5;
	}
	States
	{
	Spawn:
		FAXE FGHIJ 3;
	Death:
		FAXE K 3;
		Stop;
	}
}

	
