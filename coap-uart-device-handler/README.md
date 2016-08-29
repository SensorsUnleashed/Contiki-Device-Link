# COAP Proxy uart
This is a module, that will make the board ask through uart which resources to accept. This way the radio is interchangeable to all sensors.
Perhaps, later we will have some basic capabilities included directly into the core radio module.

All actions are forwarded to the attached through the uart, and like wise, all readings delivered the other way around.

## Protocol description

Messages are SLIP encoded, and all messages ends with the END charater (0xC0)

| SEQ NO | CMD | PAYLOAD | LENGTH | CRC16 |
|--------|:---:|---------|--------|------:|
| 1 byte | 1 byte | 1 byte | 1 byte | 2 bytes|

* SEQ NO: Will be counting up for each message
* CMD: Indicates what is to be done. Will be further described later on.
* LENGTH: Tells the length of the entire message untill the end of the payload.
* PAYLOAD: The data that is need for the command issued.
* CRC16: Contains the checksum for all bytes from SEQ NO to end of PAYLOAD.



### Commands (Send from radio to attached device):

#### *0x01 Request number of resource*

Returns:
* Length: 1
* payload: A number for all the resource available

#### *0x02 Request resource*
For each resource served by the client, the resource can be queried by sending the requested index in the payload.

Returns payload:
* resource index
* url - eg. "actuators/toggle"
* flags
	* 0x01 Get
	* 0x02 Set
* Attributes - resource type (rt): (https://tools.ietf.org/html/rfc6690#page-9)
	* 0x01 Timer
	* 0x02 Button

Message could look like:<br>
0x01, 0x02, "/button/push\0", 0x02, 0x0D, CRC16
