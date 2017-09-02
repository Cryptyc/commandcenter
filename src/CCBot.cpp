#include "sc2api/sc2_api.h"
#include "rapidjson/document.h"
#include "JSONTools.h"
#include "CCBot.h"
#include "Util.h"

CCBot::CCBot(std::string ConfigFileLocation)
    : m_map(*this)
    , m_bases(*this)
    , m_unitInfo(*this)
    , m_workers(*this)
    , m_gameCommander(*this)
    , m_strategy(*this)
	, m_config(ConfigFileLocation)
{
    
}

void CCBot::OnGameStart() 
{
    m_config.readConfigFile();

    // get my race
    auto playerID = Observation()->GetPlayerID();
    for (auto & playerInfo : Observation()->GetGameInfo().player_info)
    {
        if (playerInfo.player_id == playerID)
        {
            m_playerRace[Players::Self] = playerInfo.race_actual;
        }
        else
        {
            m_playerRace[Players::Enemy] = playerInfo.race_requested;
        }
    }

    m_strategy.onStart();
    m_map.onStart();
    m_unitInfo.onStart();
    m_bases.onStart();
    m_workers.onStart();

    m_gameCommander.onStart();
}

void CCBot::OnStep()
{
    Control()->GetObservation();

    m_map.onFrame();
    m_unitInfo.onFrame();
    m_bases.onFrame();
    m_workers.onFrame();
    m_strategy.onFrame();

    m_gameCommander.onFrame();

    Debug()->SendDebug();
}

// TODO: Figure out my race
const sc2::Race & CCBot::GetPlayerRace(int player) const
{
    BOT_ASSERT(player == Players::Self || player == Players::Enemy, "invalid player for GetPlayerRace");
    return m_playerRace[player];
}

BotConfig & CCBot::Config()
{
     return m_config;
}

const MapTools & CCBot::Map() const
{
    return m_map;
}

const StrategyManager & CCBot::Strategy() const
{
    return m_strategy;
}

const BaseLocationManager & CCBot::Bases() const
{
    return m_bases;
}

const UnitInfoManager & CCBot::UnitInfo() const
{
    return m_unitInfo;
}

WorkerManager & CCBot::Workers()
{
    return m_workers;
}

const sc2::Unit * CCBot::GetUnit(const UnitTag & tag) const
{
    return Observation()->GetUnit(tag);
}

sc2::Point2D CCBot::GetStartLocation() const
{
    return Observation()->GetStartLocation();
}

void CCBot::OnError(const std::vector<sc2::ClientError> & client_errors, const std::vector<std::string> & protocol_errors)
{
    
}

void *CreateNewAgent(std::string ConfigFileLocation)
{
	return (void *) new CCBot(ConfigFileLocation);
}

int GetAgentRace(std::string ConfigFileLocation)
{
	rapidjson::Document doc;

	std::string config = JSONTools::ReadFile(ConfigFileLocation);

	if (config.length() == 0)
	{
		return -1;
	}
	bool parsingFailed = doc.Parse(config.c_str()).HasParseError();
	if (parsingFailed)
	{
		return -1;
	}
	if (doc.HasMember("Game Info") && doc["Game Info"].IsObject())
	{
		std::string botRaceString;
		const rapidjson::Value & info = doc["Game Info"];
		JSONTools::ReadString("BotRace", info, botRaceString);
		return (int)BotConfig::GetRace(botRaceString);
	}
	return -1;
}

const char *GetAgentName(std::string ConfigFileLocation)
{
	rapidjson::Document doc;

	std::string config = JSONTools::ReadFile(ConfigFileLocation);

	if (config.length() == 0)
	{
		return nullptr;
	}
	bool parsingFailed = doc.Parse(config.c_str()).HasParseError();
	if (parsingFailed)
	{
		return nullptr;
	}

	if (doc.HasMember("Bot Info") && doc["Bot Info"].IsObject())
	{
		std::string BotName;
		const rapidjson::Value & info = doc["Bot Info"];
		JSONTools::ReadString("BotName", info, BotName);
		return BotName.c_str();
	}
	return nullptr;

}