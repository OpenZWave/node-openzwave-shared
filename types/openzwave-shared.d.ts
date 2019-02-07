declare module "openzwave-shared" {
	namespace ZWave {
		export type ValueType =
			| "bool"
			| "byte"
			| "decimal"
			| "int"
			| "list"
			| "schedule"
			| "short"
			| "string"
			| "button"
			| "raw"
			| "max";
		export type ValueGenre = "basic" | "user" | "system" | "config" | "count";

		export interface NodeInfo {
			manufacturer: string;
			manufacturerid: string;
			product: string;
			producttype: string;
			productid: string;
			type: string;
			name: string;
			loc: string;
		}

		export enum Notification {
			MessageComplete = 0,
			Timeout = 1,
			Nop = 2,
			NodeAwake = 3,
			NodeSleep = 4,
			NodeDead = 5,
			NodeAlive = 6
		}

		export enum ControllerState {
			Normal = 0,
			Starting = 1,
			Cancel = 2,
			Error = 3,
			Waiting = 4,
			Sleeping = 5,
			InProgress = 6,
			Completed = 7,
			Failed = 8,
			NodeOK = 9,
			NodeFailed = 10
		}

		export enum LogLevel {
			NoLogging = 0,
			AllMessages = 1,
			FatalMessagesOnly = 2,
			ErrorMessagesAndHigher = 3,
			WarningMessagesAndHigher = 4,
			AlertMessagesAndHigher = 5,
			InfoMessagesAndHigher = 6,
			DetailedMessagesAndHigher = 7,
			DebugMessagesAndHigher = 8,
			ProtocolInformationAndHigher = 9
		}

		export interface DriverStats {
			SOFCnt: number;
			ACKWaiting: number;
			readAborts: number;
			badChecksum: number;
			readCnt: number;
			writeCnt: number;
			CANCnt: number;
			NAKCnt: number;
			ACKCnt: number;
			OOFCnt: number;
			dropped: number;
			retries: number;
			callbacks: number;
			badroutes: number;
		}

		export interface NodeStats {
			sentCnt: number;
			sentFailed: number;
			retries: number;
			receivedCnt: number;
			receivedDups: number;
			receivedUnsolicited: number;
			lastRequestRTT: number;
			lastResponseRTT: number;
			averageRequestRTT: number;
			averageResponseRTT: number;
			quality: number;
			sentTS: string;
			receivedTS: string;
		}

		export interface ValueId {
			node_id: number;
			class_id: number;
			instance: number;
			index: number;
		}

		export interface Value<T = boolean | number | string> {
			value_id: string;
			node_id: number;
			class_id: number;
			type: ValueType;
			genre: ValueGenre;
			instance: number;
			index: number;
			label: string;
			units: string;
			help: string;
			read_only: boolean;
			write_only: boolean;
			min: number;
			max: number;
			is_polled: boolean;
			values?: string[];
			value: T;
		}

		export interface SwitchPoint {
			hours: number;
			minutes: number;
			setback: number;
		}

		export interface SceneInfo {
			sceneid: number;
			label: string;
		}

		export interface IConstructorParameters {
			/**
			 * This is the directory location where various files created by the library are stored. Examples include the zwcfg_.xml and LogFiles_
			 */
			UserPath: string;
			/**
			 * This is the directory where the device database resides
			 */
			ConfigPath: string;
			/**
			 * Enable Logging in the Library or not
			 */
			Logging: boolean;
			/**
			 * Enable log output to stdout (or console)
			 */
			ConsoleOutput: boolean;
			/**
			 * The Log File Name to use (will be output in the UserPath Directory
			 */
			LogFileName: string;
			/**
			 * On Restart, should we erase old log files, or append to existing log files
			 */
			AppendLogFile: boolean;
			/**
			 *
			 */
			SaveLogLevel: LogLevel;
			/**
			 *
			 */
			QueueLogLevel: ZWave.LogLevel;
			/**
			 *
			 */
			DumpTriggerLevel: ZWave.LogLevel;
			/**
			 * Automatically Associate the Control with any Groups that have be flagged by specific devices in the Device Database
			 */
			Associate: boolean;
			/**
			 * When a Message Transaction (via the Zwave Protocol) has been completed, receive a Code_MsgComplete notification via the Notification Interface.
			 */
			NotifyTransactions: boolean;
			/**
			 * Maximum Attempts the Library will try to Initialize the controller
			 */
			DriverMaxAttempts: number;
			/**
			 * When Shutting Down, should the library automatically save the Network Configuration in zwcfg_.xml_
			 */
			SaveConfiguration: boolean;
			/**
			 * How long we should spend polling the entire network, or how long between polls we should wait. (See IntervalBetweenPolls)
			 */
			PollInterval: number;
			/**
			 * Should the above value be how long to wait between polling the network devices, or how often all devices should be polled
			 */
			IntervalBetweenPools: boolean;
			/**
			 * After Processing a Group Changed Notification, should we update the Return Routes Map on affected devices
			 */
			PerformReturnRoutes: boolean;
			/**
			 * Specify Which Command Classes the Library will support
			 */
			Include: string;
			/**
			 * Specifically Exclude Command Classes from the Library
			 */
			Exclude: string;
			/**
			 * After a Value is Refreshed, should we send a notification to the application
			 */
			SuppressValueRefresh: boolean;
			/**
			 * Timeout before retrying to send a message. Defaults to 40 Seconds
			 */
			RetryTimeout: number;
			/**
			 * Network Key to use for Encrypting Secure Messages over the Network
			 */
			NetworkKey: Buffer;
			/**
			 * Automatically become a SUC if there is No SUC on the network
			 */
			EnableSIS: boolean;
			/**
			 * Assume Devices that support the Wakeup Class are awake when starting up OZW
			 */
			AssumeAwake: boolean;
			/**
			 * Should we refresh all UserCodes in the UserCode CC when we start up
			 */
			RefreshAllUserCodes: boolean;
		}
	}

	/**
	 * Open ZWave NodeJS wrapper object interface.
	 */
	class ZWave {
		constructor(settings: Partial<ZWave.IConstructorParameters>);

		on(event: "connected", listener: (version: string) => void): this;
		on(event: "driver ready", listener: (homeId: number) => void): this;
		on(event: "driver failed", listener: () => void): this;
		on(
			event:
				| "polling enabled"
				| "polling disabled"
				| "node removed"
				| "node added",
			listener: (nodeId: number) => void
		): this;
		on(
			event: "node event",
			listener: (nodeId: number, data: any) => void
		): this;
		on(
			event: "node ready" | "node naming" | "node available",
			listener: (nodeId: number, nodeInfo: ZWave.NodeInfo) => void
		): this;
		on(
			event: "value added" | "value changed",
			listener: (nodeId: number, comClass: number, value: ZWave.Value) => void
		): this;
		on(
			event: "value removed",
			listener: (nodeId: number, comClass: number, index: number) => void
		): this;

		on(
			event: "notification",
			listener: (nodeId: number, notification: ZWave.Notification) => void
		): this;
		on(event: "scan complete", listener: () => void): this;
		on(
			event: "controller command",
			listener: (
				nodeId: number,
				state: ZWave.ControllerState,
				notif: number,
				message: string
			) => void
		): this;
		on(event: string, listener: (...args: any[]) => void): this;

		// Exposed by "openzwave-config.cc"

		setConfigParam(
			nodeId: number,
			param: number,
			value: number,
			size?: number
		): void;
		requestConfigParam(nodeId: number, param: number): void;
		requestAllConfigParams(nodeId: number): void;

		// Exposed by "openzwave-driver.cc"

		/**
		 * @param {string} path Usually a name of COM port, such as /dev/ttyACM0
		 */
		connect(path: string): void;

		/**
		 * @param {string} path Usually a name of COM port, such as /dev/ttyACM0
		 */
		disconnect(path: string): void;

		/**
		 * Reset the ZWave controller chip.  A hard reset is destructive and wipes
		 * out all known configuration, a soft reset just restarts the chip.
		 */
		hardReset(): void;

		softReset(): void;

		/**
		 * Return Node ID of the controller itself.
		 */
		getControllerNodeId(): number;

		getSUCNodeId(): number;

		/**
		 * Query if the controller is a primary controller. The primary controller
		 * is the main device used to configure and control a Z-Wave network.
		 * There can only be one primary controller - all other controllers
		 * are secondary controllers.
		 */
		isPrimaryController(): boolean;

		/**
		 * Query if the controller is a static update controller. A Static
		 * Update Controller (SUC) is a controller that must never be moved
		 * in normal operation and which can be used by other nodes to
		 * receive information about network changes.
		 */
		isStaticUpdateController(): boolean;

		/**
		 * Query if the controller is using the bridge controller library.
		 * A bridge controller is able to create virtual nodes that can be
		 * associated with other controllers to enable events to be passed on.
		 */
		isBridgeController(): boolean;

		/**
		 * Get the version of the Z-Wave API library used by a controller.
		 */
		getLibraryVersion(): string;

		/**
		 * Get a string containing the Z-Wave API library type used by a controller.
		 *
		 * The controller should never return a slave library type. For a
		 * more efficient test of whether a controller is a Bridge Controller,
		 * use the IsBridgeController method.
		 */
		getLibraryTypeName():
			| "Static Controller"
			| "Controller"
			| "Enhanced Slave"
			| "Slave"
			| "Installer"
			| "Routing Slave"
			| "Bridge Controller"
			| "Device Under Test";

		getSendQueueCount(): number;

		// Exposed by "openzwave-groups.cc"
		/**
		 * Gets the number of association groups reported by this node.
		 * In Z-Wave, groups are numbered starting from one. For example,
		 * if a call to GetNumGroups returns 4, the _groupIdx value to use
		 * in calls to GetAssociations, AddAssociation and RemoveAssociation
		 * will be a number between 1 and 4.
		 */
		getNumGroups(): number;

		getAssociations(nodeId: number, groupIdx: number): Array<number>;

		getMaxAssociations(nodeId: number, groupIdx: number): number;

		getGroupLabel(nodeId: number, groupIdx: number): string;

		addAssociation(nodeId: number, groupIdx: number, tgtNodeId: number): void;

		removeAssociation(
			nodeId: number,
			groupIdx: number,
			tgtNodeId: number
		): void;

		// Exposed by "openzwave-management.cc"

		/**
		 * Start the Inclusion Process to add a Node to the Network.
		 * The Status of the Node Inclusion is communicated via Notifications.
		 * Specifically, you should monitor ControllerCommand Notifications.
		 */
		addNode(doSecurity?: boolean): boolean;

		/**
		 * Remove a Device from the Z-Wave Network
		 * The Status of the Node Removal is communicated via Notifications.
		 * Specifically, you should monitor ControllerCommand Notifications.
		 */
		removeNode(): boolean;

		/**
		 * Check if the Controller Believes a Node has Failed.
		 * This is different from `thevIsNodeFailed` call in that we test the Controllers
		 * Failed Node List, where as `vtheIsNodeFailed` is testing our list of Failed Nodes,
		 * which might be different. The Results will be communicated via Notifications.
		 * Specifically, you should monitor the ControllerCommand notifications.
		 */
		hasNodeFailed(nodeId: number): boolean;

		/**
		 * Ask a Node to update its Neighbor Tables
		 * This command will ask a Node to update its Neighbor Tables.
		 */
		requestNodeNeighborUpdate(nodeId: number): boolean;

		/**
		 * Ask a Node to delete all Return Route.
		 * This command will ask a Node to delete all its return routes, and will
		 * rediscover when needed.
		 */
		deleteAllReturnRoutes(nodeId: number): boolean;

		/**
		 * Send a NIF frame from the Controller to a Node.
		 * This command send a NIF frame from the Controller to a Node.
		 */
		sendNodeInformation(nodeId: number): boolean;

		/**
		 * Create a new primary controller when old primary fails. Requires SUC.
		 * This command Creates a new Primary Controller when the Old Primary has Failed.
		 * Requires a SUC on the network to function.
		 */
		createNewPrimary(): boolean;

		/**
		 * Receive network configuration information from primary controller. Requires
		 * secondary. This command prepares the controller to receive Network Configuration from a Secondary Controller.
		 */
		receiveConfiguration(): boolean;

		/**
		 * Replace a failed device with another.
		 * If the node is not in the controller's failed nodes list, or the node responds,
		 * this command will fail. You can check if a Node is in the Controllers Failed
		 * node list by using the HasNodeFailed method.
		 */
		replaceFailedNode(nodeId: number): boolean;

		/**
		 * Add a new controller to the network and make it the primary.
		 * The existing primary will become a secondary controller.
		 */
		transferPrimaryRole(): boolean;

		/**
		 * Update the controller with network information from the SUC/SIS.
		 */
		requestNetworkUpdate(nodeId: number): boolean;

		/**
		 * Send information from primary to secondary.
		 */
		replicationSend(nodeId: number): boolean;

		/**
		 * Create a handheld button id.
		 */
		createButton(nodeId: number, buttonId: number): boolean;

		/**
		 * Delete a handheld button id.
		 */
		deleteButton(nodeId: number, buttonId: number): boolean;

		/**
		 * LEGACY MODE (using BeginControllerCommand)
		 * @obsolete
		 */
		beginControllerCommand(command: any): void;

		cancelControllerCommand(): void;

		writeConfig(): void;

		getDriverStatistics(): ZWave.DriverStats;

		getNodeStatistics(nodeId: number): ZWave.NodeStats;

		// Exposed by "openzwave-network.cc"

		/**
		 * Test network node. Sends a series of messages to a network node
		 * for testing network reliability.
		 */
		testNetworkNode(nodeId: number, retries?: number): void;

		testNetwork(retries?: number): void;

		/**
		 * Heal network by requesting node's rediscover their neighbors.
		 * Sends a ControllerCommand_RequestNodeNeighborUpdate to every node.
		 * Can take a while on larger networks.
		 */
		healNetwork(doRR?: boolean): void;

		/**
		 * Heal network node by requesting the node rediscover their neighbors.
		 */
		healNetworkNode(nodeId: number, doRR?: boolean): void;

		// Exposed by "openzwave-nodes.cc"

		/**
		 * Gets the neighbors for a node.
		 */
		getNodeNeighbors(nodeId: number): Array<number>;

		setNodeOn(nodeId: number): void;

		setNodeOff(nodeId: number): void;

		/**
		 * Generic dimmer control
		 */
		setNodeLevel(nodeId: number, level: number): void;

		switchAllOn(): void;

		switchAllOff(): void;

		pressButton(valueId: ZWave.ValueId): void;

		releaseButton(valueId: ZWave.ValueId): void;

		/**
		 * Write a new location string to the device, if supported.
		 */
		setNodeLocation(nodeId: number, location: string): void;

		/**
		 * Write a new name string to the device, if supported.
		 */
		setNodeName(nodeId: number, name: string): void;

		setNodeManufacturerName(nodeId: number, name: string): string;

		setNodeProductName(nodeId: number, productName: string): string;

		/**
		 * Trigger the fetching of fixed data about a node. Causes the node's data to be obtained from the
		 * Z-Wave network in the same way as if it had just been added.
		 * This method would normally be called automatically by OpenZWave, but if you know that a node has been
		 * changed, calling this method will force a refresh of all of the data held by the library.
		 * This can be especially useful for devices that were asleep when the application was first run.
		 * This is the same as the query state starting from the beginning.
		 */
		refreshNodeInfo(nodeId: number): void;

		requestNodeState(nodeId: number): void;

		requestNodeDynamic(nodeId: number): void;

		isNodeListeningDevice(nodeId: number): boolean;

		isNodeFrequentListeningDevice(nodeId: number): boolean;

		isNodeBeamingDevice(nodeId: number): boolean;

		isNodeRoutingDevice(nodeId: number): boolean;

		isNodeSecurityDevice(nodeId: number): boolean;

		getNodeMaxBaudRate(nodeId: number): number;

		getNodeVersion(nodeId: number): number;

		getNodeSecurity(nodeId: number): number;

		getNodeBasic(nodeId: number): number;

		getNodeGeneric(nodeId: number): number;

		getNodeSpecific(nodeId: number): number;

		getNodeType(nodeId: number): string;

		getNodeProductName(nodeId: number): string;

		getNodeName(nodeId: number): string;

		getNodeLocation(nodeId: number): string;

		getNodeManufacturerId(nodeId: number): string;

		getNodeProductType(nodeId: number): string;

		getNodeProductId(nodeId: number): string;

		// Exposed by "openzwave-values.cc"

		/**
		 * Generic value set.
		 */
		setValue(
			nodeid: number,
			class_id: number,
			instance: number,
			index: number,
			value: string | boolean | number
		): void;
		setValue(valueId: ZWave.ValueId, value: string | boolean | number): void;

		/**
		 * Refresh value from Z-Wave network.
		 */
		refreshValue(valueId: ZWave.ValueId): boolean;

		/**
		 * Ask OZW to verify the value change before notifying the application.
		 */
		setChangeVerified(valueId: ZWave.ValueId, enabled: boolean): void;
		setChangeVerified(
			nodeid: number,
			class_id: number,
			instance: number,
			index: number,
			enabled: boolean
		): void;
		/**
		 * Get number of thermostat switch points.
		 */
		getNumSwitchPoints(valueId: ZWave.ValueId): number;

		getSwitchPoint(valueId: ZWave.ValueId, index: number): ZWave.SwitchPoint;

		clearSwitchPoints(valueId: ZWave.ValueId): void;

		removeSwitchPoint(
			valueId: ZWave.ValueId,
			switchPoint: ZWave.SwitchPoint
		): void;

		// Exposed by "openzwave-polling.cc"

		/**
		 * Get the time period between polls of a node's state, in ms.
		 */
		getPollInterval(): number;

		/**
		 * Set the time period between polls of a node's state. Due to patent concerns, some devices do not report
		 * state changes automatically to the controller. These devices need to have their state polled at
		 * regular intervals. The length of the interval is the same for all devices.
		 * To even out the Z-Wave network traffic generated by polling, OpenZWave divides the polling interval by
		 * the number of devices that have polling enabled, and polls each in turn.
		 * It is recommended that if possible, the interval should not be set shorter than the number of polled devices
		 * in seconds (so that the network does not have to cope with more than one poll per second).
		 */
		setPollInterval(intervalMilliseconds: number): void;

		/**
		 * Enable polling on a ValueID.
		 */
		enablePoll(valueId: ZWave.ValueId, intensity?: number): boolean;

		/**
		 * Disable polling on a ValueID.
		 */
		disablePoll(valueId: ZWave.ValueId): boolean;

		/**
		 * Determine the polling of a device's state.
		 */
		isPolled(valueId: ZWave.ValueId): boolean;

		setPollIntensity(valueId: ZWave.ValueId, intensity: number): void;

		getPollIntensity(valueId: ZWave.ValueId): number;

		// Exposed by "openzwave-scenes.cc"

		/**
		 * Returns sceneId.
		 */
		createScene(label: string): number;
		removeScene(sceneId: number): void;
		getScenes(): Array<ZWave.SceneInfo>;

		addSceneValue(sceneId: number, value: ZWave.ValueId): void;
		removeSceneValue(sceneId: number, value: ZWave.ValueId): void;
		sceneGetValues(sceneId: number): Array<ZWave.ValueId>;
		activateScene(sceneId: number): void;
	}

	export = ZWave;
}
