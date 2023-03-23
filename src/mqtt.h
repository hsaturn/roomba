#include "module.h"
#include "TinyMqtt.h"

class Mqtt : public Module
{
	public:
    Mqtt();

		const char* name() const override { return "mqtt"; }
    MqttClient* client() { return client_; }

    void loop() override;

    MqttBroker* broker_;
    MqttClient* client_;
    
};