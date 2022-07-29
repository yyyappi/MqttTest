package mqtt;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.sql.Timestamp;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

public class MqttSubscriber implements MqttCallback {
	Timestamp recieveTime;
	Timestamp lastTime;
	String broker = "";
	String topic = "";

	public MqttSubscriber(String brokerHostName, String subccriberTopic) {
		broker = "tcp://" + brokerHostName + ":1883";
		topic = subccriberTopic;


	}

	@Override
	public void connectionLost(Throwable cause) {

		System.out.println("Connection lost");
		System.exit(1);


	}

	@Override
	public void deliveryComplete(IMqttDeliveryToken arg0) {


	}

	@Override
	public void messageArrived(String topic, MqttMessage message) throws MqttException {

		System.out.println("Message arrived");
		System.out.println("Topic:" + topic);
		System.out.println("Message:" + new String(message.getPayload()));

		recieveTime = new Timestamp(System.currentTimeMillis());
		MqttThread.recieveData = new String(message.getPayload());

	}

	public boolean isNew() {
		boolean flag = false;
		if (recieveTime == lastTime)
			flag = false;
		else
			flag = true;
		lastTime = recieveTime;
		return flag;
	}

	public static void main(String[] args) throws InterruptedException {
		try {
			MqttSubscriber subscriber = new MqttSubscriber("localhost", "test/lol");
			subscriber.subscribe();

		} catch (MqttException me) {
			System.out.println("reason: " + me.getReasonCode());
			System.out.println("message: " + me.getMessage());
			System.out.println("localize: " + me.getLocalizedMessage());
			System.out.println("cause: " + me.getCause());
			System.out.println("exception: " + me);
		}
		// TODO 自動生成されたメソッド・スタブ

	}

	public void subscribe() throws MqttException, InterruptedException {
		final int qos = 2;
		final String clientId = "Subscribe";

		MqttClient client = new MqttClient(broker, clientId, new MemoryPersistence());
		client.setCallback(this);
		MqttConnectOptions connOpts = new MqttConnectOptions();
		connOpts.setCleanSession(false);

		System.out.println("Connecting to broker:" + broker);
		client.connect(connOpts);
		client.subscribe(topic, qos);

		BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
		try {
			br.readLine();

		} catch (IOException e) {
			System.exit(1);
		}
		client.disconnect();
		client.close();
		System.out.println("Disconnected");
	}

}
