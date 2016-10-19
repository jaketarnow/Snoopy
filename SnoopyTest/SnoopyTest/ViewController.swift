//
//  ViewController.swift
//  SnoopyTest
//
//  Created by Jacob Tarnow on 9/13/16.
//  Copyright © 2016 JacobTarnow. All rights reserved.
//

import UIKit
import Foundation
import SystemConfiguration.CaptiveNetwork

/* Create a class to grab the SSID */
open class SSID {
    class func getSSID() ->  String {
        var currentSSID = ""
        //Returns the names of all network interfaces Captive Network Support is monitoring
        let interfaces = CNCopySupportedInterfaces()
        //check for nil in regards to simulator... apparently works on actual device
        if interfaces != nil {
            
            let interfacesArray = Array(arrayLiteral: interfaces) //cast the interfaces into an Array
            if interfacesArray.count > 0 {
                let interfaceName =  String(describing: interfacesArray[0]) //grab the first one
                let rec = unsafeBitCast(interfaceName, to: AnyObject.self)
                //Returns the current network info for a given network interface
                //A dictionary containing the interface’s current network info.
                let unsafeInterfaceData = CNCopyCurrentNetworkInfo("\(rec)" as CFString)
                if unsafeInterfaceData != nil {
                    let interfaceData = unsafeInterfaceData! as Dictionary!
                    currentSSID = interfaceData?[kCNNetworkInfoKeySSID] as! String
                    let ssiddata = NSString(data:interfaceData![kCNNetworkInfoKeySSIDData]! as! Data, encoding:String.Encoding.utf8.rawValue) as! String
                    // ssid data from hex
                }
            }
        }
        return currentSSID
    }
}
class ViewController: UIViewController {
    
    @IBOutlet weak var ssid: UILabel!
    @IBOutlet weak var refresh: UIButton!
    
    func update(_ button: UIButton) {
        ssid.text = SSID.getSSID()
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
       
        if SSID.getSSID() == "Func not supported" {
            ssid.text = "WiFi Name"
        } else {
            ssid.text = SSID.getSSID()
        }
        print("HEREREERE", SSID.getSSID())
        
        refresh.addTarget(self, action: #selector(ViewController.update(_:)), for: .touchUpInside)
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


}

