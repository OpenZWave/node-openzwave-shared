"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var Notification;
(function (Notification) {
    // noinspection JSUnusedGlobalSymbols
    Notification[Notification["MessageComplete"] = 0] = "MessageComplete";
    Notification[Notification["Timeout"] = 1] = "Timeout";
    Notification[Notification["Nop"] = 2] = "Nop";
    Notification[Notification["NodeAwake"] = 3] = "NodeAwake";
    Notification[Notification["NodeSleep"] = 4] = "NodeSleep";
    Notification[Notification["NodeDead"] = 5] = "NodeDead";
    Notification[Notification["NodeAlive"] = 6] = "NodeAlive";
})(Notification = exports.Notification || (exports.Notification = {}));
var ControllerState;
(function (ControllerState) {
    ControllerState[ControllerState["Normal"] = 0] = "Normal";
    ControllerState[ControllerState["Starting"] = 1] = "Starting";
    ControllerState[ControllerState["Cancel"] = 2] = "Cancel";
    ControllerState[ControllerState["Error"] = 3] = "Error";
    ControllerState[ControllerState["Waiting"] = 4] = "Waiting";
    ControllerState[ControllerState["Sleeping"] = 5] = "Sleeping";
    ControllerState[ControllerState["InProgress"] = 6] = "InProgress";
    ControllerState[ControllerState["Completed"] = 7] = "Completed";
    ControllerState[ControllerState["Failed"] = 8] = "Failed";
    ControllerState[ControllerState["NodeOK"] = 9] = "NodeOK";
    ControllerState[ControllerState["NodeFailed"] = 10] = "NodeFailed";
})(ControllerState = exports.ControllerState || (exports.ControllerState = {}));
var LogLevel;
(function (LogLevel) {
    LogLevel[LogLevel["NoLogging"] = 0] = "NoLogging";
    LogLevel[LogLevel["AllMessages"] = 1] = "AllMessages";
    LogLevel[LogLevel["FatalMessagesOnly"] = 2] = "FatalMessagesOnly";
    LogLevel[LogLevel["ErrorMessagesAndHigher"] = 3] = "ErrorMessagesAndHigher";
    LogLevel[LogLevel["WarningMessagesAndHigher"] = 4] = "WarningMessagesAndHigher";
    LogLevel[LogLevel["AlertMessagesAndHigher"] = 5] = "AlertMessagesAndHigher";
    LogLevel[LogLevel["InfoMessagesAndHigher"] = 6] = "InfoMessagesAndHigher";
    LogLevel[LogLevel["DetailedMessagesAndHigher"] = 7] = "DetailedMessagesAndHigher";
    LogLevel[LogLevel["DebugMessagesAndHigher"] = 8] = "DebugMessagesAndHigher";
    LogLevel[LogLevel["ProtocolInformationAndHigher"] = 9] = "ProtocolInformationAndHigher";
})(LogLevel = exports.LogLevel || (exports.LogLevel = {}));
var MetaDataFields;
(function (MetaDataFields) {
	MetaDataFields[MetaDataFields["OzwInfoPage_URL"] = 0] = "OzwInfoPage_URL";
	MetaDataFields[MetaDataFields["ZWProductPage_URL"] = 1] = "ZWProductPage_URL";
	MetaDataFields[MetaDataFields["ProductPic"] = 2] = "ProductPic";
	MetaDataFields[MetaDataFields["Description"] = 3] = "Description";
	MetaDataFields[MetaDataFields["ProductManual_URL"] = 4] = "ProductManual_URL";
	MetaDataFields[MetaDataFields["ProductPage_URL"] = 5] = "ProductPage_URL";
	MetaDataFields[MetaDataFields["InclusionHelp"] = 6] = "InclusionHelp";
	MetaDataFields[MetaDataFields["ExclusionHelp"] = 7] = "ExclusionHelp";
	MetaDataFields[MetaDataFields["ResetHelp"] = 8] = "ResetHelp";
	MetaDataFields[MetaDataFields["WakeupHelp"] = 9] = "WakeupHelp";
	MetaDataFields[MetaDataFields["ProductSupport_URL"] = 10] = "ProductSupport_URL";
	MetaDataFields[MetaDataFields["Frequency"] = 11] = "Frequency";
	MetaDataFields[MetaDataFields["Name"] = 12] = "Name";
	MetaDataFields[MetaDataFields["Identifier"] = 13] = "Identifier";
	MetaDataFields[MetaDataFields["Invalid"] = 255] = "Invalid";
})(MetaDataFields = exports.MetaDataFields || (exports.MetaDataFields = {}));
