export enum Notification {
	// noinspection JSUnusedGlobalSymbols
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
