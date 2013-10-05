#import "RemoteViewController.h"
#import "SensorsController.h"

@interface RemoteViewController ()

-(IBAction)powerClicked;
-(IBAction)homeClicked;
-(IBAction)sourceClicked;
-(IBAction)backClicked;
-(IBAction)volumeUpClicked;
-(IBAction)volumeDownClicked;
-(IBAction)okClicked;
-(IBAction)leftClicked;
-(IBAction)rightClicked;
-(IBAction)upClicked;
-(IBAction)downClicked;

@end

@implementation RemoteViewController

-(id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    
    if (self)
    {
    }
    
    return self;
}

-(void)viewDidLoad
{
    [super viewDidLoad];
    
    
    [[SensorsController getInstance] startDeviceSearch];
}

-(IBAction)powerClicked
{
    [[[SensorsController getInstance] connectedRemote] sendPower];
}

-(IBAction)homeClicked
{
    [[[SensorsController getInstance] connectedRemote] sendHome];
}

-(IBAction)sourceClicked
{
    [[[SensorsController getInstance] connectedRemote] sendSource];
}

-(IBAction)backClicked
{
    [[[SensorsController getInstance] connectedRemote] sendBack];
}

-(IBAction)volumeUpClicked
{
    [[[SensorsController getInstance] connectedRemote] sendVolumeUp];
}

-(IBAction)volumeDownClicked
{
    [[[SensorsController getInstance] connectedRemote] sendVolumeDown];
}

-(IBAction)okClicked
{
    [[[SensorsController getInstance] connectedRemote] sendOk];
}

-(IBAction)leftClicked
{
    [[[SensorsController getInstance] connectedRemote] sendLeft];
}

-(IBAction)rightClicked
{
    [[[SensorsController getInstance] connectedRemote] sendRight];
}

-(IBAction)upClicked
{
    [[[SensorsController getInstance] connectedRemote] sendUp];
}

-(IBAction)downClicked
{
    [[[SensorsController getInstance] connectedRemote] sendDown];
}

@end
