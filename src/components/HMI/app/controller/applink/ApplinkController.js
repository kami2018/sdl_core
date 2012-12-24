/**
 * @name MFT.ApplinkController
 * 
 * @desc Main Applink Controller
 * 
 * @category	Controller
 * @filesource	app/controller/applink/ApplinkController.js
 * @version		1.0
 *
 * @author		Artem Petrosyan
 */
 
MFT.ApplinkController = Em.Object.create({
	
	/**
	 * List of registered applications
	 * @type object
	 */
	registeredApps: {
	},
	
	/** 
	 * List of Applink application models
	 *
	 * @type object
	 */
	applicationModels: {
		0:	'ApplinkMediaModel',
		1:	'ApplinkNonMediaModel'
	},
	
	/**
	 * Register application
	 * @desc ...
	 *
	 * @param applicationId: Number
	 * @param applicationType: Number
	 */
	registerApplication: function( applicationId, applicationType ) {
		if ( this.registeredApps[ applicationId ] ) {
			//Em.Logger.error('Application ['+ applicationId +'] already registered!');
			return;
		}
		
		this.registeredApps[ applicationId ] = applicationType;
		//Em.Logger.log('Application ['+ applicationId +'] registered!');
	},

	/**
	 * Unregister application
	 * @desc ...
	 *
	 * @param applicationId: Number
	 */
	unRegisterApplication: function( applicationId ) {
		delete this.registeredApps[ applicationId ];
	},

    /**
     * Applink Driver Distraction ON/OFF switcher
     * 
     * @param checked: bool
     */
    selectdDriverDistraction: function(checked){
        if(checked){
            FFW.UI.onDriverDistraction( this.eDriverDistractionState.on );
            MFT.DriverDistraction.activate();
        }else{
            FFW.UI.onDriverDistraction( this.eDriverDistractionState.off );
            MFT.DriverDistraction.deactivate();
        }
    },

    /**
     * Applink Protocol Version 2 ON/OFF switcher
     * 
     * @param checked: bool
     */
    selectProtocolVersion: function(checked){
        if(checked){
            FFW.AppLinkCoreClient.OnVersionChanged( 2 );
        }else{
            FFW.AppLinkCoreClient.OnVersionChanged( 1 );
        }
    },

	/**
	 * Get application model
	 * @desc ...
	 *
	 * @param applicationId: Number
	 * @return Object Model
	 */
	getApplicationModel: function( applicationId ) {
		return MFT[ this.applicationModels[ this.registeredApps[ applicationId ] ] ];
	},

	/* Function returns ChangeDeviceView 
	 * back to previous state
	 */
	turnChangeDeviceViewBack: function(){
        MFT.States.goToState('info.apps');
    },

	/**
	 * Enter screen vith list of devices application model
	 * 
	 */
	onGetDeviceList: function(  ) {
		MFT.States.goToState('info.devicelist');
		FFW.AppLinkCoreClient.getDeviceList();
	},

	/**
	 * Send notification if device was choosed
	 *
	 * @param element: MFT.Button
	 */
	onDeviceChoosed: function( element ) {
		FFW.UI.OnDeviceChosen( element.deviceName );
		MFT.ApplinkModel.set( 'deviceName', element.deviceName );
		this.turnChangeDeviceViewBack();
	},

	/**
	 * Method creates list of Application ID's
	 * Then call HMI method for display a list of Applications
	 */
	onGetAppList: function( appList ){
		for(var i = 0; i < appList.length; i++){
			if( appList[i].isMediaApplication ){
				MFT.ApplinkController.registerApplication(appList[i].appId, 0);
			}else{
				MFT.ApplinkController.registerApplication(appList[i].appId, 1);
			}
		}
		MFT.ApplinkModel.onGetAppList( appList );
	},

	/**
	 * Method call's request to get list of applications
	 */
	findNewApps: function(){
		FFW.AppLinkCoreClient.getAppList();
	},

	/**
	 *  Method activates selected registered application
	 */
	onActivateApplinkApp: function(element){
		this.getApplicationModel(element.appId).turnOnApplink( element.appName, element.appId );
	}

});