package main;

import mqtt.MqttPublisher;
import mqtt.MqttThread;

public class testMqttClient {

	//メインメソッド(ここからはじまる)

	public static void main(String[] args) {
		MqttThread mthread=new MqttThread("localhost", "test/lol");
		mthread.start();

		MqttPublisher publisher=new MqttPublisher("localhost", "test/lol");
		publisher.publish("HelloTestmainjava");
		if(mthread.isNew()) {
			System.out.println(mthread.recieveData);

		}
	}

}
