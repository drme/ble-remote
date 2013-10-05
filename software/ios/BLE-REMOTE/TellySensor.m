#import "TellySensor.h"
#import "Utils.h"

@implementation TellySensor
{
    CBPeripheral* device;
    CBCharacteristic* encodingTypeCharacteristic;
    CBCharacteristic* addressTypeCharacteristic;
    CBCharacteristic* commandCharacteristic;
    CBCharacteristic* batteryLevelCharacteristic;
}

@synthesize batteryLevel;
@synthesize rssiLevel;
@synthesize deviceName;

-(CBUUID*)toCBUUID:(int)value
{
    int serviceUUID = value;

    UInt16 c = [Utils swap:serviceUUID];

    NSData* cd = [[NSData alloc] initWithBytes:(char*)&c length:2];

    CBUUID* su = [CBUUID UUIDWithData:cd];

    return su;
}

-(void)startServicesSeach
{
    id tellyService = [CBUUID UUIDWithString:TELLY_SERVICE_ID];
    id batteryService = [CBUUID UUIDWithString:@"180f"];
    
    
    NSArray* services = [NSArray arrayWithObjects:tellyService, batteryService, nil];
    
    [self->device discoverServices:services];
}

-(id)initWithData:(CBPeripheral*)peripheral
{
    self = [super init];
    
    if (self)
    {
        self->device = peripheral;
        self->device.delegate = self;

        [self startServicesSeach];
    }
    
    return self;
}

-(void)sendCommand:(uint8_t)command;
{
    NSData* data = [NSData dataWithBytes:&command length:sizeof(command)];
    
    [device writeValue:data forCharacteristic:commandCharacteristic type:CBCharacteristicWriteWithResponse];
}

-(void)sendPower
{
    [self sendCommand:21];
}

-(void)sendHome
{
    [self sendCommand:96];
}

-(void)sendSource
{
    [self sendCommand:37];
}

-(void)sendBack
{
    [self sendCommand:96];
}

-(void)sendVolumeUp
{
    [self sendCommand:18];
}

-(void)sendVolumeDown
{
    [self sendCommand:19];
}

-(void)sendOk
{
    [self sendCommand:101];
}

-(void)sendLeft
{
    [self sendCommand:52];
}

-(void)sendRight
{
    [self sendCommand:51];
}

-(void)sendUp
{
    [self sendCommand:116];
}

-(void)sendDown
{
    [self sendCommand:117];
}

-(void)updateStatus
{
    if (nil != batteryLevelCharacteristic)
    {
        [device readValueForCharacteristic:batteryLevelCharacteristic];
    }
    
    [device readRSSI];
    
    self.rssiLevel = [device.RSSI intValue];
    self.deviceName = [deviceName description];
}

-(CBPeripheral*)getDevice
{
    return device;
}

-(void)peripheralDidUpdateName:(CBPeripheral *)peripheral
{
}

- (void)peripheralDidInvalidateServices:(CBPeripheral*)peripheral
{
}

-(void)peripheralDidUpdateRSSI:(CBPeripheral*)peripheral error:(NSError*)error
{
    NSLog(@"RSSI: %d", (int)peripheral.RSSI);
}

-(void)peripheral:(CBPeripheral*)peripheral didDiscoverServices:(NSError*)error
{
    if (!error)
    {
        for (int i = 0; i < peripheral.services.count; i++)
        {
            CBService* service = [peripheral.services objectAtIndex:i];
            
            [peripheral discoverCharacteristics:nil forService:service];
        }
    }
}

-(void)peripheral:(CBPeripheral*)peripheral didDiscoverIncludedServicesForService:(CBService*)service error:(NSError*)error
{
}

-(void)peripheral:(CBPeripheral*)peripheral didDiscoverCharacteristicsForService:(CBService*)service error:(NSError*)error
{
    if (!error)
    {
        for (int i = 0; i < service.characteristics.count; i++)
        {
            CBCharacteristic* characteristic = [service.characteristics objectAtIndex:i];
            
            UInt16 uuid = [Utils CBUUIDToInt:characteristic.UUID];
            
            switch (uuid)
            {
                case 65505:
                    commandCharacteristic = characteristic;
                    break;
                case 0x2a19:
                    batteryLevelCharacteristic = characteristic;
                    break;
                default:
                    NSLog(@"Unhandled characteristic: %f", (float)uuid);
                    break;
            }
        }
    }
}

-(void)peripheral:(CBPeripheral*)peripheral didUpdateValueForCharacteristic:(CBCharacteristic*)characteristic error:(NSError*)error
{
    if (error)
    {
        return;
    }
    
    if (characteristic == batteryLevelCharacteristic)
    {
        int16_t temp = 0;
        
        [characteristic.value getBytes:&temp length:sizeof(temp)];
        
        batteryLevel = temp;
    }
}

-(void)peripheral:(CBPeripheral*)peripheral didWriteValueForCharacteristic:(CBCharacteristic*)characteristic error:(NSError*)error
{
    if (error)
    {
        return;
    }
}

-(void)peripheral:(CBPeripheral*)peripheral didUpdateNotificationStateForCharacteristic:(CBCharacteristic*)characteristic error:(NSError*)error
{
}

-(void)peripheral:(CBPeripheral*)peripheral didDiscoverDescriptorsForCharacteristic:(CBCharacteristic*)characteristic error:(NSError*)error
{
}

-(void)peripheral:(CBPeripheral*)peripheral didUpdateValueForDescriptor:(CBDescriptor*)descriptor error:(NSError*)error
{
}

-(void)peripheral:(CBPeripheral*)peripheral didWriteValueForDescriptor:(CBDescriptor*)descriptor error:(NSError*)error
{
}

@end
