#import "ViewController.h"
#import "Device.h"
#import "ScanLAN.h"

@interface ViewController () <NSURLSessionDelegate, NSURLSessionDataDelegate>

@property (nonatomic) CFAbsoluteTime startTime;
@property (nonatomic) CFAbsoluteTime stopTime;
@property (nonatomic) long long bytesReceived;
@property (nonatomic, copy) void (^speedTestCompletionHandler)(CGFloat megabytesPerSecond, NSError *error);
@property (weak, nonatomic) IBOutlet UITableView *tableView;
@property NSMutableArray *connctedDevices;
@property ScanLAN *lanScanner;
@property NSMutableDictionary *dict;
@property NSString *theSpeed;
@property CGFloat speed;

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    UIBarButtonItem *refreshBarButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemRefresh target:self action:@selector(startScanningLAN)];
    self.navigationItem.rightBarButtonItem = refreshBarButton;
}

- (void)viewDidAppear:(BOOL)animated {
    [self startScanningLAN];
}

- (void)viewWillDisappear:(BOOL)animated {
    [self.lanScanner stopScan];
    [self.lanScanner getUpnpDiscovery];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)startScanningLAN {
    [self.lanScanner stopScan];
    self.lanScanner = [[ScanLAN alloc] initWithDelegate:self];
    self.connctedDevices = [[NSMutableArray alloc] init];
    [self.lanScanner startScan];
    [self.lanScanner getUpnpDiscovery];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Return the number of sections.
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Return the number of rows in the section.
    return self.connctedDevices.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier forIndexPath:indexPath];
    
    //add subclass for table view cell with button in it 
    
    Device *device = [self.connctedDevices objectAtIndex:indexPath.row];
    cell.textLabel.text = device.name;
    cell.detailTextLabel.text = device.address;
    
    return cell;
}


- (void)testDownloadSpeedWithTimout:(NSTimeInterval)timeout completionHandler:(nonnull void (^)(CGFloat megabytesPerSecond, NSError * _Nullable error))completionHandler {
    NSURL *url = [NSURL URLWithString:@"https://srollins.cs.usfca.edu/images/sami_purple.png"];
    
    self.startTime = CFAbsoluteTimeGetCurrent();
    self.stopTime = self.startTime;
    self.bytesReceived = 0;
    self.speedTestCompletionHandler = completionHandler;
    
    NSURLSessionConfiguration *configuration = [NSURLSessionConfiguration ephemeralSessionConfiguration];
    configuration.timeoutIntervalForResource = timeout;
    NSURLSession *session = [NSURLSession sessionWithConfiguration:configuration delegate:self delegateQueue:nil];
    [[session dataTaskWithURL:url] resume];
}

- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask didReceiveData:(NSData *)data {
    self.bytesReceived += 65536;
    self.stopTime = CFAbsoluteTimeGetCurrent();
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error {
    CFAbsoluteTime elapsed = 50;
    self.speed = elapsed != 0 ? 65536 / (CFAbsoluteTimeGetCurrent() - self.startTime) / 1024.0 / 1024.0 : -1;
    
    // treat timeout as no error (as we're testing speed, not worried about whether we got entire resource or not
    
    if (error == nil || ([error.domain isEqualToString:NSURLErrorDomain] && error.code == NSURLErrorTimedOut)) {
        self.speedTestCompletionHandler(self.speed, nil);
    } else {
        self.speedTestCompletionHandler(self.speed, error);
    }
}

- (IBAction)BtnClicked:(id)sender
{
    __block CGFloat msgSpeed;
    [self testDownloadSpeedWithTimout:5.0 completionHandler:^(CGFloat megabytesPerSecond, NSError *error) {
        msgSpeed = megabytesPerSecond;
        NSLog(@"IN HERERERE with speed: %@", self.theSpeed);
    }];
    
    NSIndexPath *indexPath = [self.tableView indexPathForCell:(UITableViewCell *)[[sender superview] superview]];
    Device *device = [self.connctedDevices objectAtIndex:indexPath.row];
    NSString *test = device.name;
    NSString *speedMsg =[NSString stringWithFormat:@"Speed is %f", msgSpeed];
    NSString *diagIp = [NSString stringWithFormat:@"Diagnostics for %@", test];
    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:diagIp
                                                    message:speedMsg
                                                   delegate:nil
                                          cancelButtonTitle:@"OK"
                                          otherButtonTitles:nil];
    [alert show];
}



/*
 // Override to support conditional editing of the table view.
 - (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
 {
 // Return NO if you do not want the specified item to be editable.
 return YES;
 }
 */


  //Override to support editing the table view.
 - (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
 {
 if (editingStyle == UITableViewCellEditingStyleDelete) {
  //Delete the row from the data source
 [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
 }
 else if (editingStyle == UITableViewCellEditingStyleInsert) {
  //Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
 }
 }


/*
 // Override to support rearranging the table view.
 - (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath
 {
 }
 */

/*
 // Override to support conditional rearranging of the table view.
 - (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath
 {
 // Return NO if you do not want the item to be re-orderable.
 return YES;
 }
 */

/*
 #pragma mark - Navigation
 
 // In a story board-based application, you will often want to do a little preparation before navigation
 - (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
 {
 // Get the new view controller using [segue destinationViewController].
 // Pass the selected object to the new view controller.
 }
 
 */
#pragma mark LAN Scanner delegate method
- (void)scanLANDidFindNewAdrress:(NSString *)address havingHostName:(NSString *)hostName {
    NSLog(@"found  %@", address);
    Device *device = [[Device alloc] init];
    device.name = hostName;
    device.address = address;
    [self.connctedDevices addObject:device];
    [self.tableView reloadData];
}

- (void)scanLANDidFinishScanning {
    NSLog(@"Scan finished");

    self.dict = [[NSMutableDictionary alloc] initWithCapacity:50];
    
    [self.dict setObject:self.connctedDevices forKey:@"Connected Devices"];
    
//    NSData *dataSave = [NSKeyedArchiver archivedDataWithRootObject:self.dict];
//    [[NSUserDefaults standardUserDefaults] setObject:dataSave forKey:@"test"];
//    [[NSUserDefaults standardUserDefaults] synchronize];

    
    [[[UIAlertView alloc] initWithTitle:@"Scan Finished" message:[NSString stringWithFormat:@"Number of devices connected to the Local Area Network : %d", self.connctedDevices.count] delegate:nil cancelButtonTitle:@"Ok" otherButtonTitles:nil] show];
}
@end
