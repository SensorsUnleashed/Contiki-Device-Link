
1. When a node is first powered up, it will join the network
2. It will go through all stored paires, and ask to observe those.

What happens if it a device can't pair with a node?

First it will try, if that fails, it will observe the root device for "/su/detect". The ip it is looking for will be put as payload in the observe message.

The root will put the device as well as the resource in question in a list for observing ip's.

detect_s{
  ip to look for
  oberserver structure ptr
}

The root will first look if the device is in its list of nodes. If its there, it will ping it. If there is no response to the ping, we try again in 1 minute. If it still does not respond, we will know that the device is off, and we wont ping again. When the node gets powered, RPL will make sure that the root will be informed - by then we can respond back to the resource requesting the detect.

If the node is not even in the list of nodes, it means it has not been seen ever (or at least not since the root's last reboot). We will wait till it comes back up, like after a ping.

It is not necessary to use a normal ping. A ping could just be to coap device called ping.
