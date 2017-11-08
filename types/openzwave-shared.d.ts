declare module 'openzwave-shared' {
	interface NodeInfo {
		manufacturer: string;
		// noinspection SpellCheckingInspection
		manufacturerid: string;
		product: string;
		// noinspection SpellCheckingInspection
		producttype: string;
		// noinspection SpellCheckingInspection
		productid: string;
		type: string;
		name: string;
		loc: string;
	}

	enum Notification {
		// noinspection JSUnusedGlobalSymbols
		MessageComplete = 0,
		Timeout = 1,
		Nop = 2,
		NodeAwake = 3,
		NodeSleep = 4,
		NodeDeal = 5,
		NodeAlive = 6
	}

	interface DriverStats {
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
		// noinspection SpellCheckingInspection
		badroutes: number;
	}

	interface NodeStats {
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

	interface Value {
		value_id: string;
		node_id: number;
		class_id: number;
		type: string;
		genre: string;
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
	}

	interface ValueId {
		nodeid: number;
		class_id: number;
		instance: number;
		index: number;
	}

	interface SwitchPoint {
		hours: number;
		minutes: number;
		setback: number;
	}

	interface SceneInfo {
		// noinspection SpellCheckingInspection
		sceneid: number;
		label: string;
	}

	export = ZWave;

	/**
	 * Open ZWave NodeJS wrapper object interface.
	 */
	class ZWave {
		constructor(settings: any);

		on(event: string, cb: Function);

		// !!!
		on(event: 'driver ready', cb: (homeId: string) => {});
		on(event: 'driver failed', cb: () => {});
		on(event: 'node added', cb: (nodeId: number) => {});
		on(event: 'node event', cb: (nodeId: number, data: any) => {});
		on(event: 'value added' | 'value changed', cb: (nodeId: number, comClass: string, value: any) => {});
		on(event: 'value removed', cb: (nodeId: number, comClass: string, index: any) => {});
		on(event: 'node ready', cb: (nodeId: number, nodeInfo: NodeInfo) => {});
		on(event: 'notification', cb: (nodeId: number, notification: Notification) => {});
		on(event: 'scan complete', cb: () => {});
		on(event: 'controller command', cb: (nodeId: number, retVal: number, state: number, message: string) => {});

		// !!!

		// Exposed by "openzwave-config.cc"

		setConfigParam(nodeId: number, param: number, value: number, size?: number): void;
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

		on(event: 'connected', cb: (/** Version of OZW lib */ version: string) => {});

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
		getLibraryTypeName(): 'Static Controller' | 'Controller' | 'Enhanced Slave' | 'Slave' | 'Installer' |
			'Routing Slave' | 'Bridge Controller' | 'Device Under Test';

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

		addAssociation(nodeId: number, groupIdx: number, tgtNodeId: number);

		removeAssociation(nodeId: number, groupIdx: number, tgtNodeId: number);

		// Exposed by "openzwave-management.cc"

		/**
		 * Start the Inclusion Process to add a Node to the Network.
		 * The Status of the Node Inclusion is communicated via Notifications.
		 * Specifically, you should monitor ControllerCommand Notifications.
		 */
		addNode(doSecurity: boolean = false): boolean;

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
		beginControllerCommand(command: any);

		cancelControllerCommand(): void;

		writeConfig(): void;

		getDriverStatistics(): DriverStats;

		getNodeStatistics(nodeId: number): NodeStats;

		// Exposed by "openzwave-network.cc"

		/**
		 * Test network node. Sends a series of messages to a network node
		 * for testing network reliability.
		 */
		testNetworkNode(nodeId: number, retries: number = 1): void;

		testNetwork(retries: number = 1): void;

		/**
		 * Heal network by requesting node's rediscover their neighbors.
		 * Sends a ControllerCommand_RequestNodeNeighborUpdate to every node.
		 * Can take a while on larger networks.
		 */
		healNetwork(doRR: boolean = false): void;

		/**
		 * Heal network node by requesting the node rediscover their neighbors.
		 */
		healNetworkNode(nodeId: number, doRR: boolean = false): void;

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

		pressButton(valueId: ValueId): void;

		releaseButton(valueId: ValueId): void;

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
		setValue(valueId: ValueId, value: Value): void;

		/**
		 * Refresh value from Z-Wave network.
		 */
		refreshValue(valueId: ValueId): boolean;

		/**
		 * Ask OZW to verify the value change before notifying the application.
		 */
		setChangeVerified(valueId: ValueId): void;

		/**
		 * Get number of thermostat switch points.
		 */
		getNumSwitchPoints(valueId: ValueId): number;

		getSwitchPoint(valueId: ValueId, index: number): SwitchPoint;

		clearSwitchPoints(valueId: ValueId): void;

		removeSwitchPoint(valueId: ValueId, switchPoint: SwitchPoint): void;

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
		enablePoll(valueId: ValueId, intensity: number): boolean;

		/**
		 * Disable polling on a ValueID.
		 */
		disablePoll(valueId: ValueId): boolean;

		/**
		 * Determine the polling of a device's state.
		 */
		isPolled(valueId: ValueId): boolean;

		setPollIntensity(valueId: ValueId, intensity: number): void;

		getPollIntensity(valueId: ValueId): number;

		// Exposed by "openzwave-scenes.cc"

		/**
		 * Returns sceneId.
		 */
		createScene(label: string): number;

		removeScene(sceneId: number): void;

		// noinspection SpellCheckingInspection
		getScenes(): Array<SceneInfo>;

		addSceneValue(sceneId: number, value: ValueId): void;
		removeSceneValue(sceneId: number, value: ValueId): void;
		sceneGetValues(sceneId: number): Array<ValueId>;
		activateScene(sceneId: number): void;
	}
}
