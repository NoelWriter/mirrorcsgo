#pragma once
#include "..\Utils\DrawManager.h"
#include "..\Utils\GlobalVars.h"
#include "..\Settings.h"
#include <deque>

#define PI 3.14159265358979323846f

struct lbyRecords
{
	int tick_count;
	float lby;
	Vector headPosition;
};

class BackTrack
{
	int latest_tick;
	bool IsTickValid(int tick);

public:

	lbyRecords records[64];
	void legitBackTrack(CUserCmd * cmd);
	void SaveRecord(C_BaseEntity * pEnt);
	void RageBackTrack(CUserCmd * cmd, C_BaseEntity * pEnt);
};

inline Vector angle_vector(Vector meme)
{
	auto sy = sin(meme.y / 180.f * static_cast<float>(PI));
	auto cy = cos(meme.y / 180.f * static_cast<float>(PI));

	auto sp = sin(meme.x / 180.f * static_cast<float>(PI));
	auto cp = cos(meme.x / 180.f* static_cast<float>(PI));

	return Vector(cp*cy, cp*sy, -sp);
};

inline float distance_point_to_line(Vector Point, Vector LineOrigin, Vector Dir)
{
	auto PointDir = Point - LineOrigin;

	auto TempOffset = PointDir.Dot(Dir) / (Dir.x*Dir.x + Dir.y*Dir.y + Dir.z*Dir.z);
	if (TempOffset < 0.000001f)
		return FLT_MAX;

	auto PerpendicularPoint = LineOrigin + (Dir * TempOffset);

	return (Point - PerpendicularPoint).Length();
};

struct backtrackData {
	float simtime;
	Vector hitboxPos;
};

struct rageBacktrackData {
	void SaveRecord(C_BaseEntity * pEnt);
	float simtime;
	Vector hitboxPos;
	float lowerBodyYaw;
	float velocity;
};


extern backtrackData l_SavedTicks[64][25];
extern rageBacktrackData r_SavedTicks[64][25];
extern BackTrack* backtracking;