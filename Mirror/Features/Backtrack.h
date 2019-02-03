#pragma once
#include "..\Utils\DrawManager.h"
#include "..\Utils\GlobalVars.h"
#include "..\Settings.h"
#include <deque>
#include <array>

#define PI 3.14159265358979323846f

struct LayerRecord
{
	LayerRecord()
	{
		m_nOrder = 0;
		m_nSequence = 0;
		m_flWeight = 0.f;
		m_flCycle = 0.f;
	}

	LayerRecord(const LayerRecord &src)
	{
		m_nOrder = src.m_nOrder;
		m_nSequence = src.m_nSequence;
		m_flWeight = src.m_flWeight;
		m_flCycle = src.m_flCycle;
	}

	uint32_t m_nOrder;
	uint32_t m_nSequence;
	float_t m_flWeight;
	float_t m_flCycle;
};


struct LagRecord
{
	LagRecord()
	{
		m_iPriority = -1;

		m_flSimulationTime = -1.f;
		m_vecOrigin.Init();
		m_angAngles.Init();
		m_vecMins.Init();
		m_vecMax.Init();
		m_bMatrixBuilt = false;
	}

	bool operator==(const LagRecord &rec)
	{
		return m_flSimulationTime == rec.m_flSimulationTime;
	}

	void SaveRecord(C_BaseEntity *player);

	matrix3x4_t	matrix[128];
	bool m_bMatrixBuilt;
	Vector m_vecHeadSpot;
	float m_iTickCount;

	// For priority/other checks
	int32_t m_iPriority;
	Vector  m_vecVelocity;
	float_t m_flPrevLowerBodyYaw;
	std::array<float_t, 24> m_arrflPrevPoseParameters;
	Vector  m_vecLocalAimspot;
	bool    m_bNoGoodSpots;

	// For backtracking
	float_t m_flSimulationTime;
	int32_t m_nFlags;
	Vector m_vecOrigin;	   // Server data, change to this for accuracy
	Vector m_vecAbsOrigin;
	QAngle m_angAngles;
	Vector m_vecMins;
	Vector m_vecMax;
	std::array<float_t, 24> m_arrflPoseParameters;
	std::array<LayerRecord, 15> m_LayerRecords;
};

class BackTrack
{
	int latest_tick;
	bool IsTickValid(int tick);

public:
	void legitBackTrack(CUserCmd * cmd);
	void RageBackTrack();

	void RunTicks(C_BaseEntity * target, CUserCmd * usercmd, Vector & aim_point, bool & hitchanced);

	int GetPriorityLevel(C_BaseEntity * player, LagRecord * lag_record);

	void ProcessCMD(int iTargetIdx, CUserCmd * usercmd);

	void RemoveBadRecords(int Idx, std::deque<LagRecord>& records);

	bool StartLagCompensation(C_BaseEntity * player);

	void FinishLagCompensation(C_BaseEntity * player);

	bool FindViableRecord(C_BaseEntity * player, LagRecord * record);

	bool CheckTarget(int i);

	std::deque<LagRecord> m_LagRecord[64];
	std::deque<LagRecord> backtrack_records;
	std::pair<LagRecord, LagRecord> m_RestoreLagRecord[64];
	LagRecord current_record[64];

	inline void ClearHistory()
	{
		for (int i = 0; i < 64; i++)
			m_LagRecord[i].clear();
	}
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
	float simtime;
	Vector hitboxPos;
	float lowerBodyYaw;
	float velocity;
};


extern backtrackData l_SavedTicks[64][25];
extern rageBacktrackData r_SavedTicks[64][25];
extern BackTrack* backtracking;