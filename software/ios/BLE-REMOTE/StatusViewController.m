#import "StatusViewController.h"

@interface StatusViewController ()

@property (weak, nonatomic) IBOutlet UITextField* sensorName;
@property (weak, nonatomic) IBOutlet UITextField* batteryLevel;
@property (weak, nonatomic) IBOutlet UITextField* rssiValue;
@property (weak, nonatomic) IBOutlet UIProgressView* batteryStatus;
@property (weak, nonatomic) IBOutlet UISwitch* connectSwitch;
@property (weak, nonatomic) IBOutlet UIBarButtonItem* connectButton;
@property (weak, nonatomic) IBOutlet UIActivityIndicatorView* activityView;

-(IBAction)connectClicked;

@end

@implementation StatusViewController

@synthesize batteryStatus;
@synthesize sensorName;
@synthesize batteryLevel;
@synthesize rssiValue;
@synthesize connectSwitch;
@synthesize connectButton;
@synthesize activityView;

-(void)refreshStatus
{
    if (nil != [[SensorsController getInstance] connectedRemote])
    {
        [[[SensorsController getInstance] connectedRemote] updateStatus];
    
        float level = (float)[[[SensorsController getInstance] connectedRemote] batteryLevel] / 100.0f;
    
        batteryStatus.progress = level;
        batteryLevel.text = [NSString stringWithFormat:@"%f V - %0.2f%%", 2.0f + level, level * 100.0f];
        
        rssiValue.text = [NSString stringWithFormat:@"%f db", (float)[[SensorsController getInstance] connectedRemote].rssiLevel];
        
        
    }
}

-(void)viewDidLoad
{
    [super viewDidLoad];
    
    [SensorsController getInstance].delegateDiscover = self;
    
    id timer = [NSTimer scheduledTimerWithTimeInterval:10 target:self selector:@selector(refreshStatus) userInfo:nil repeats:YES];
    
    [timer fire];
    
    [activityView startAnimating];
    
    [[SensorsController getInstance] startDeviceSearch];
    
    if (nil != [SensorsController getInstance].connectedRemote)
    {
        [self connected];
    }
}

-(void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
}

-(void)connected
{
    [activityView stopAnimating];
    activityView.hidden = TRUE;
    
    connectSwitch.on = TRUE;
    connectButton.enabled = FALSE;
}

-(void)disconnected
{
    connectSwitch.on = FALSE;
    connectButton.enabled = TRUE;
    [[SensorsController getInstance] startDeviceSearch];
}

-(IBAction)connectClicked
{
    if (TRUE == connectSwitch.on)
    {
        if (nil == [[SensorsController getInstance] connectedRemote])
        {
            activityView.hidden = FALSE;
            [activityView startAnimating];
            [[SensorsController getInstance] startDeviceSearch];
        }
    }
    else
    {
        if (nil != [[SensorsController getInstance] connectedRemote])
        {
            [[SensorsController getInstance] disconnectActive];
        }
    }
}

@end
