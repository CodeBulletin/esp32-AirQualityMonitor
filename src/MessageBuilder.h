#ifndef MessageBuilder_h
#define MessageBuilder_h

class MessageBuilder
{
    private:
        String message;
    public:
        MessageBuilder(String time, int deviceId, int type = 0);   
        MessageBuilder& addSensor(int sensorId) {
            // if message last has , remove it
            if (message[message.length() - 1] == ',') {
                message = message.substring(0, message.length() - 1);
            }
            message += "/" + String(sensorId) + ":";
            return *this;
        }
        MessageBuilder& addAttribute(String name, float value) {
            message += name + "=" + String(value) + ",";
            return *this;
        }
        MessageBuilder& addAttribute(String name, int value) {
            message += name + "=" + String(value) + ",";
            return *this;
        }
        MessageBuilder& addAttribute(String name, String value) {
            message += name + "=" + value + ",";
            return *this;
        }
        MessageBuilder& end() {
            // if message last has , remove it
            if (message[message.length() - 1] == ',') {
                message = message.substring(0, message.length() - 1);
            }
            return *this;
        }
        MessageBuilder& addMessage(String message) {
            this->message += "/" + message;
            return *this;
        }
        const char* c_str() {
            return message.c_str();
        }
};

MessageBuilder::MessageBuilder(String time, int deviceId, int type) {
    message = (type == 0 ? "r:": "m:") + time + "/" + String(deviceId);
}


#endif // MessageBuilder_h