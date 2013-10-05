#import "SensorsController.h"
#import "Utils.h"
#import "TellySensor.h"

@implementation SensorsController
{
    CBCentralManager* centralManager;
    NSMutableArray* peripherals;
}

@synthesize delegateDiscover;
@synthesize connectedRemote;
@synthesize connectingPeripheral;

-(id)init
{
    self = [super init];
    
    if (self)
    {
        self->centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
    }
    
    return self;
}

+(SensorsController*)getInstance
{
    static SensorsController *sharedSingleton;
    
    @synchronized(self)
    {
        if (!sharedSingleton)
        {
            sharedSingleton = [[SensorsController alloc] init];
        }
        
        return sharedSingleton;
    }
}

-(void)scanTimer:(NSTimer*)timer
{
 //   [self->centralManager stopScan];
}

-(void)startDeviceSearch
{
    [self->peripherals removeAllObjects];
    
    if (CBCentralManagerStatePoweredOn != self->centralManager.state)
    {
        return;
    }
    
    NSArray* services = [[NSArray alloc] initWithObjects:[CBUUID UUIDWithString:TELLY_SERVICE_ID], nil];
    
    [NSTimer scheduledTimerWithTimeInterval:(float)15.0 target:self selector:@selector(scanTimer:) userInfo:nil repeats:NO];
    
    NSDictionary* options = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:NO], CBCentralManagerScanOptionAllowDuplicatesKey, nil];

    [self->centralManager scanForPeripheralsWithServices:services options:options];
}

-(void)connected
{
    [self->delegateDiscover connected];
}

-(void)disconnected
{
    [self->delegateDiscover disconnected];
}

-(void)centralManagerDidUpdateState:(CBCentralManager*)central
{
    if (centralManager.state == CBPeripheralManagerStatePoweredOn)
    {
        [self startDeviceSearch];
    }
}

-(void)centralManager:(CBCentralManager *)central didRetrievePeripherals:(NSArray *)peripherals
{
}

-(void)centralManager:(CBCentralManager *)central didRetrieveConnectedPeripherals:(NSArray *)peripherals
{
}

-(void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI
{
    NSLog(@"Discovered peripheral: %@ advertisement %@ RSSI: %@", [peripheral description], [advertisementData description], [RSSI description]);

    if (peripheral.isConnected == FALSE)
    {
        connectingPeripheral = peripheral;
        [centralManager connectPeripheral:peripheral options:nil];
    }
}

-(void)centralManager:(CBCentralManager*)central didConnectPeripheral:(CBPeripheral*)peripheral
{
    connectedRemote = [[TellySensor alloc] initWithData:peripheral];

    [self connected];
}

-(void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error
{
}

-(void)centralManager:(CBCentralManager*)central didDisconnectPeripheral:(CBPeripheral*)peripheral error:(NSError*)error
{
    connectedRemote = nil;
    
    [self disconnected];
    
    [Utils showMessage:@"Disconnected"];
}

-(void)disconnectActive
{
    [self->centralManager cancelPeripheralConnection:[self->connectedRemote getDevice]];

    connectedRemote = nil;
}

@end
