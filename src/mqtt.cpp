#include "mqtt.h"
#include "nanos.h"

extern Roomba roomba;
extern OutputStream telnet;

void onPublish(const MqttClient* /* source */, const Topic& topic, const char* payload, size_t /* length */)
{
  std::string t = topic.str();
  std::string device = Utils::getWord(t, '/');

  telnet << "mqtt: received " << topic.c_str() << ", " << payload << endl;
  std::string cmd(payload);

  Module::Params p(roomba, cmd, telnet);
  nanos->execute(p);
}

Mqtt::Mqtt()
: broker_(new MqttBroker(1883)), client_(new MqttClient(broker_))
{
  broker_->begin();
  client_->setCallback(onPublish);
  client_->subscribe("#");

  handlers = {
    { "publish" ,  { "topic payload", [this](Params& p)->bool
      {
        Topic topic(getWord(p.args));
        std::string payload(getWord(p.args));
        if (topic.str().length()) client_->publish(topic, payload, true);
        return true;
      }}},
    { "retain", { "size : set retain size of broker", [this](Params& p)->bool
      {
        broker_->retain(getInt(p.args));
        return true;
      }}},
  };
}

void Mqtt::loop()
{
  broker_->loop();
  client_->loop();
}
