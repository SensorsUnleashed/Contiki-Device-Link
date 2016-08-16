This is a module, that will make the board ask through uart what our capabilities are. This way the radio is interchangeable to all sensors.
Perhaps, later we will have some basic capabilities included directly into the core radio module.

Uart will act as proxy for the 

1. Request rescources from the uart
2. Publish all available rescources 
3. All actions is forwarded to the uart, and responses returned 


Protocol description

Messages are SLIP encoded, and all messages ends with the END charater (0xC0)

SEQ NO, CMD, PAYLOAD, LENGTH, CRC16

SEQ NO is 1 byte. Will be counting up for each message
CMD is 1 byte. Indicates what is to be done. Will be further described later on.
LENGTH is 1 byte. Tells the length of the entire message untill the end of the payload.
PAYLOAD is the data that is need for the command issued.
CRC16 is a 2 byte field. Contains the checksum for all bytes from SEQ NO to end of PAYLOAD.

Commands (Send from radio to attached device):

0x01 Request number of resource
	
	Return:
		SEQ NO: Same SEQ NO as host
		CMD: Same CMD as host
		Length: 1
		payload: 1 byte. A number for all the resource available
		CRC16
		
0x02 Request resource number
	Payload: resource number
	
	Return payload:
	url - eg. button, relay etc
	flags - defined by rest_resource_flags_t ( METHOD_GET, METHOD_POST etc)
	
