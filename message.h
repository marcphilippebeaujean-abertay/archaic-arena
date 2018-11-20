#pragma once
#include <SFML\Network.hpp>
class message
{
public:
	message();
	~message();

	enum MESSAGE_TYPE
	{
		TCP = 0,
		UDP = 1,
		RECV = 2
	};

	// setters
	void unwrapPacket(sf::Packet msgPacket);
	void initPacketValues(sf::Int16 cmdID, sf::Vector2f msgVec, sf::Int64 msgValue, sf::Int16 clientID, float messageTime, MESSAGE_TYPE msgType, float sendTime);
	void setMsgType(MESSAGE_TYPE msgType) { this->msgType = msgType; }
	void setLatency(float latencyTime) { latencyTime = latencyTime; }

	// getters
	sf::Packet createPacket();
	sf::Vector2f getMessageVector() { return msgVec; }
	sf::Int16 getCmdID() { return commandID; }
	sf::Vector2f getMsgVec() { return msgVec; }
	sf::Int64 getMsgVal() { return msgValue; }
	sf::Int16 getClientID() { return clientID; }
	float getLatency() { return latencyTime; }
	float getTimeSent() { return sendTime; }
	MESSAGE_TYPE getMessageType() { return msgType; }

private:
	// data to be sent
	sf::Int16 commandID;
	sf::Vector2f msgVec;
	sf::Int64 msgValue;
	sf::Int16 clientID;
	float latencyTime;
	float sendTime;

	MESSAGE_TYPE msgType;
};

