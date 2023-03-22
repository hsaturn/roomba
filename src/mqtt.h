#include "command.h"
#include "TinyMqtt.h"

class Mqtt : public Command
{
	public:
    Mqtt();

		const char* name() const override { return "mqtt"; }
    MqttClient* client() { return client_; }

    void loop() override;

    MqttBroker* broker_;
    MqttClient* client_;
    
};