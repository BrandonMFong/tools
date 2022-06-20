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
					boolean cont = true;
					Enumeration<InetAddress> iaEnum;

					try {
						// Get next interface
						ni = niEnum.nextElement();
						if (ni == null) {
							result = 1;
						}
					} catch (NoSuchElementException e) {
						result = 1;
						PrintException(e);
					}

					// Retrieve the next interface's mac address
					if (result == 0) {
						mac = ni.getHardwareAddress();
						cont = mac != null ? true : false;
					}

					if (cont) {
						if (result == 0) {
							String[] hexadecimal = new String[mac.length];

							// Get the bytes from the address
							for (int i = 0; i < mac.length; i++) {
								hexadecimal[i] = String.format("%02X", mac[i]);
							}

							// Combine each byte element by ':'
							String macAddress = String.join(":", hexadecimal);
							
							// Print the mac address
							System.out.println(macAddress);

							// Since this interface has a mac address, let's get
							// the ip address
							iaEnum = ni.getInetAddress();
							if (iaEnum == null) {
								result = 1;
								System.out.println("Could not get inet address");
							} else {
								while (iaEnum.hasMoreElements() && (result == 0)) {
									InetAddress ia = null;
								}
							}
						}
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

