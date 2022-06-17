// Your First Program
/**
 * author: Brando
 * date: 6/17/22
 */

import java.util.Enumeration;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.lang.Throwable;
import java.util.NoSuchElementException;

class netinfo {
	static void PrintException(Throwable e) {
		System.out.println("[ " + e.toString() + " ]");
		System.out.println("Message: " + e.getMessage());
		System.out.println("Stack trace:");
		e.printStackTrace();
	}

	public static void main(String[] args) {
		int result = 0;
		Enumeration<NetworkInterface> niEnum = null;

		try {
			// Init the enum
			niEnum = NetworkInterface.getNetworkInterfaces();

			if (niEnum == null) {
				result = 1;
				System.out.println("Could not get the network interfaces");
			} else {
				// Sweep the interfaces
				while (niEnum.hasMoreElements() && (result == 0)) {
					NetworkInterface ni = null;
					byte[] mac = null;

					try {
						ni = niEnum.nextElement();
						if (ni == null) {
							result = 1;
						}
					} catch (NoSuchElementException e) {
						result = 1;
						PrintException(e);
					}

					if (result == 0) {
						mac = ni.getHardwareAddress();
					}

					if (mac != null) {
						String[] hexadecimal = new String[mac.length];
						for (int i = 0; i < mac.length; i++) {
							hexadecimal[i] = String.format("%02X", mac[i]);
						}
						String macAddress = String.join(":", hexadecimal);
						System.out.println(macAddress);
					}
				}
			}
		} catch (SocketException e) {
			result = 1;
			PrintException(e);
		}

		System.exit(result);
	}
}

