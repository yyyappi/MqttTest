package mqtt;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;


public class MqttPublisher {
	String broker="";
	String topic="";


	public MqttPublisher(String brokerHostName,String publishTopic) {
		broker="tcp://"+brokerHostName+":1883";
		topic=publishTopic;

	}

	public void publish(String publishMessage) {
		final int qos =2;
		final String clientId="Publisher";

		try {
			MqttClient mqttClient =new MqttClient(broker,clientId,new MemoryPersistence());
			MqttConnectOptions connOpts=new MqttConnectOptions();
			connOpts.setCleanSession(false);

			mqttClient.connect(connOpts);

			MqttMessage message=new MqttMessage(publishMessage.getBytes());
			message.setQos(qos);

			System.out.println("publish message");
			System.out.println("Topic :"+topic+", Message :"+message);

			mqttClient.publish(topic, message);

			mqttClient.disconnect();
			mqttClient.close();


		}catch(MqttException me) {
			System.out.println("reason: " +me.getReasonCode());
			System.out.println("message:"+me.getMessage());
			System.out.println("localize:"+me.getLocalizedMessage());
			System.out.println("cause:"+me.getCause());
			System.out.println("exception:"+me);
		}



	}

	public static void main(String[] args) {
		MqttPublisher publisher=new MqttPublisher("localhost","test/lol");
		publisher.publish("MQTT test from java");
	}


}
