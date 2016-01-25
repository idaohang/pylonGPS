#pragma once

#include<QMainWindow>
#include "ui_transceiverFormFramework.h"
#include<QPushButton>
#include<QStackedWidget>
#include <QtSvg/QSvgWidget>
#include<marble/MarbleWidget.h>
#include<marble/AbstractFloatItem.h>
#include<QThread>
#include "IPBasedLocationRetriever.hpp"
#include "addressBasedLocationRetriever.hpp"
#include<marble/GeoDataLatLonAltBox.h>
#include<marble/GeoDataDocument.h>
#include<marble/GeoDataPlacemark.h>
#include<marble/GeoDataLineString.h>
#include<marble/GeoDataTreeModel.h>
#include<marble/MarbleModel.h>
#include<marble/MarbleWidgetInputHandler.h>
#include<map>
#include<mutex>
#include <QLineEdit>
#include "base_station_stream_information.pb.h"
#include "casterBasestationWidget.hpp"
#include "casterBasestationDataReceiverWidget.hpp"
#include "transceiver.hpp"
#include "createDataReceiverDialogWindow.hpp"
#include "fileDataReceiverWidget.hpp"
#include "tcpDataReceiverWidget.hpp"
#include "zmqDataReceiverWidget.hpp"
#include "dataSenderDialogWindow.hpp"
#include "fileDataSenderWidget.hpp"
#include "tcpDataSenderWidget.hpp"
#include "zmqDataSenderWidget.hpp"
#include "basestationDataSenderWidget.hpp"
#include "tranceiver_configuration.pb.h"

//Kludgy hack to be able to connect Qt objects across namespaces. Apologies for using the using command in a header.  However, this isn't library code and the name is quite specific.
using Marble::GeoDataLatLonAltBox;

namespace pylongps
{

//In microseconds
const Poco::Timestamp::TimeDiff TRANSCEIVER_GUI_EXPIRATION_TIME = 60000000; //A minute
const Poco::Timestamp::TimeDiff TRANSCEIVER_GUI_MINIMUM_TIME_BETWEEN_QUERIES = 1000000; //1 second
const double TRANSCEIVER_QUERY_CACHING_CONSTANT = 5.0;
//const double VISIBLE_BASESTATION_FUDGE_FACTOR = 1.5; //How far our of the visible range is rendered
const int64_t PORT_NUMBER_FOR_BASESTATION_REGISTRATION = 10001;
const int64_t MAX_NUMBER_OF_SENDERS_ADDED_TO_RECEIVER = 1000000;
const std::string TRANSCEIVER_CONFIGURATION_FILE_EXTENSION = ".pylonTransceiverConfiguration";

/**
This class is the main window used with the pylon GPS 2.0 tranceiver.  It is written to use Qt4 with the Marble GIS library because that is what is currently available in the Ubuntu repositories.
*/
class transceiverGUI : public QMainWindow, public Ui::transceiverFormFramework
{
Q_OBJECT

public:

/**
This function initializes the class, connecting widgets, setting up the form generated by Qt designer and adding the components that the designer doesn't handle well.

@throw: This function can throw exceptions
*/
transceiverGUI();

/**
This function adds a data receiver widget to be displayed, inserting it into all common data receiver maps and updating the related displays.  It also takes ownership of the widget.
@param inputDataReceiverConnectionString: The data receiver connection string associated with the data receiver
@param inputDataReceiverWidget: The data receiver widget to display/take ownership of
*/
void addDataReceiverWidgetToDisplay(const std::string &inputDataReceiverConnectionString, QWidget *inputDataReceiverWidget);

/**
This function adds a data sender widget to be displayed, inserting it into all common data sender maps and updating the related displays.  It also takes ownership of the widget.
@param inputDataReceiverConnectionString: The connection string for the receiver this sender is associated with
@param inputDataSenderIDString: The data sender ID associated with the data sender
@param inputDataSenderWidget: The data sender widget to display/take ownership of
*/
void addDataSenderWidgetToDisplay(const std::string &inputDataReceiverConnectionString, const std::string &inputDataSenderIDString, QWidget *inputDataSenderWidget);

std::unique_ptr<zmq::context_t> context;
std::string casterIPString;

std::unique_ptr<transceiver> receiverSender;

std::string casterRequestSocketConnectionString;

Marble::MarbleWidget *mapWidget; //Pointer to map display
std::unique_ptr<QSvgWidget> receiverSenderConnectionsSVGWidget;

std::string configurationFilePath;


//Map view updating related info
Poco::Timestamp tranceiverGUIStartTime;
std::array<double, 2> mapCenter; //Latitude, longitude
std::array<double, 4> currentMapBoundaries;
std::set<std::pair<int64_t, int64_t> > lastSetOfVisibleBasestations;

//Data receiver stuff
std::map<std::string, std::unique_ptr<QWidget> > dataReceiverConnectionStringToOwningPointer;
std::map<int64_t, QWidget *> displayOrderToDataReceiverWidgetAddress; //Used to preserve data sender display order
int64_t nextDataReceiverWidgetDisplayOrder = 0;
std::map<std::string, int64_t> dataReceiverConnectionStringToDisplayOrderForAssociatedSenders; //Decrements associated int each time a sender is added for a receiver

std::map<std::pair<int64_t, int64_t>, std::string> selectedBasestationIDToAssociatedZMQDataReceiver;

//Other data receiver related info
std::map<std::string, std::string> fileDataReceiverConnectionStringToAssociatedFileName;
std::map<std::string, std::pair<std::string, int64_t>> tcpDataReceiverConnectionStringToServerIPAddressAndPort;
std::map<std::string, std::pair<std::string, int64_t>> zmqDataReceiverConnectionStringToZMQPubIPAddressAndPort;

//Data sender related stuff
std::multimap<std::string, std::string > dataReceiverConnectionStringToConnectedSenderIDs;
std::map<std::string, std::unique_ptr<QWidget> > dataSenderIDToOwningPointer;
std::map<int64_t, QWidget *> displayOrderToDataSenderWidgetAddress; //Used to preserve data sender display order

std::map<std::string, std::string> fileDataSenderIDStringToAssociatedPath;
std::map<std::string, int64_t> tcpDataSenderIDStringToServerPort;
std::map<std::string, int64_t> zmqDataSenderIDStringToZMQPubPort;
std::map<std::string, std::tuple<std::string, double, double, corrections_message_format, double, std::string, credentials, bool>> basestationDataSenderIDToDetails; //ID -> informal name, latitude, longitude, message format, expected update rate, secret key, credentials, if credentials are valid or not

//Map related info
std::map<std::pair<int64_t, int64_t>, std::unique_ptr<Marble::GeoDataPlacemark> > basestationIDToMapPlacemark;
std::map<Marble::GeoDataPlacemark *, std::pair<int64_t, int64_t> > mapPlacemarkToBasestationID;

//Basestation display widgets info
std::map<std::pair<int64_t, int64_t>, casterBasestationWidget*> selectableBasestationsWidgets; 




//Mutex protected BASE STATION DATA MODEL////////////////////
std::mutex visibleBasestationMutex;
std::map<std::pair<int64_t, int64_t>, base_station_stream_information> potentiallyVisibleBasestationList;
std::array<double, 4> lastQueryBoundaryInRadians; //west, north, east, south
Poco::Timestamp timeOfLastQueryUpdate;
Poco::Timestamp timeOfLastSentQuery; //When the last query request was sent out

//Sorted key sets to allow catched updates by searching for keys which are currently visible
std::multimap<double, std::pair<int64_t, int64_t> > latitudeToBasestationKey;
std::multimap<double, std::pair<int64_t, int64_t> > longitudeToBasestationKey;
//end mutex protected BASE STATION DATA MODEL////////////////////


public slots:
/**
This slot opens a dialog window to select where to save the settings file to and then calls saveCurrentConfiguration with the returned path.
*/
void openSaveCurrentConfigurationDialogWindow();

/**
This function saves the current configuration of the transceiver to a file so that it can be reconstructed later either with the GUI or a command line program.
@param inputPathToSaveTo: Where to save the configuration file
*/
void saveCurrentConfiguration(const std::string &inputPathToSaveTo);

/**
This function opens a dialog window to select a configuration file to load and then calls loadConfiguration with the selected path.
*/
void openLoadConfigurationDialogWindow();

/**
This function clears the current settings, reads a transceiver configuration file and applies the settings to this transceiver.
@param inputPathToReadFrom: The path to read from
*/
void loadConfiguration(const std::string &inputPathToReadFrom);

/**
This function updates the SVG widget to draw lines pointing from a data receiver to its associated data sender.
*/
void updateConnectionsDrawing();

/**
This function creates a basestation data sender and its associated GUI.
@param inputDataReceiverConnectionString: The connection string for the data receiver to forward from
@param inputInformalBasestationName: The informal name of the basestation
@param inputLatitude: The latitude that will be reported for the basestation
@param inputLongitude: The longitude that will be reported for the basestation
@param inputMessageFormat: The format of the messages are in
@param inputExpectedMessageRate: The expected sending rate of the messages
@param inputBasestationSecretKey: A secret key to use for an authenticated basestation (assumes unauthenticated if missing)
@param inputCredentials: The credentials to use with authentication (empty if not used)
@param inputCredentialsLoaded: True if the credentials should be valid
*/
void addBasestationDataSender(const std::string &inputDataReceiverConnectionString, const std::string &inputInformalBasestationName, double inputLatitude, double inputLongitude, corrections_message_format inputMessageFormat, double inputExpectedMessageRate, const std::string & inputBasestationSecretKey, credentials inputCredentials, bool inputCredentialsLoaded);

/**
This function creates a zmq data sender and its associated GUI.
@param inputDataReceiverConnectionString: The connection string for the data receiver to forward from
@param inputPort: The port on the host the the ZMQ server is listening to 
*/
void addZMQDataSender(const std::string &inputDataReceiverConnectionString, int64_t inputPort);

/**
This function creates a tcp data sender and its associated GUI.
@param inputDataReceiverConnectionString: The connection string for the data receiver to forward from
@param inputPort: The port on the host the the TCP server is listening to 
*/
void addTCPDataSender(const std::string &inputDataReceiverConnectionString, int64_t inputPort);

/**
This function creates a file data sender and its associated GUI.
@param inputDataReceiverConnectionString: The connection string for the data receiver to forward from
@param inputFilePath: The path to the file to send to
*/
void addFileDataSender(const std::string &inputDataReceiverConnectionString, const std::string &inputFilePath);

/**
This function opens up a dialog window to allow a new data sender to be added for a particular data receiver.
@param inputDataReceiverConnectionString: The connection string for the data receiver to forward the data from.
*/
void createDataSenderDialogWindowForDisplay(const std::string &inputDataReceiverConnectionString);

/**
This function creates a zmq data receiver and its associated GUI.
@param inputIPAddress: A string holding the IP address of the TCP server to connect to
@param inputPort: The port on the host the the ZMQ SUB port is listening to 
@return: The ZMQ connection string for the data receiver
*/
std::string addZMQDataReceiver(const std::string &inputIPAddress, int64_t inputPort);

/**
This function creates a tcp data receiver and its associated GUI.
@param inputIPAddress: A string holding the IP address of the TCP server to connect to
@param inputPort: The port on the host the the TCP server is listening to 
@return: The ZMQ connection string for the data receiver
*/
std::string addTCPDataReceiver(const std::string &inputIPAddress, int64_t inputPort);

/**
This function creates a file data receiver and its associated GUI.
@param inputFilePath: The path to the file to receive from
@return: The ZMQ connection string for the data receiver
*/
std::string addFileDataReceiver(const std::string &inputFilePath);

/**
This function adds a basestation data receiver and its associated widget.
@param inputCasterID: The ID of the caster to receive from
@param inputStreamID: The ID of the stream from the caster
@param inputBasestationInfo: The details to populate the widget with
@return: The ZMQ connection string for the data receiver or blank on failure
*/
std::string addBasestationDataReceiver(int64_t inputCasterID, int64_t inputStreamID, const base_station_stream_information &inputBasestationInfo);

/**
This function makes/destroys/updates points of interest on the map to ensure that they reflect what is stored in the base station data model.
*/
void updateMapViewAccordingToBasestationModel();

/**
This function updates any widgets which are still in the model, adds any that are new and removes all widgets which are no longer in the model.
*/
void updateSelectableBasestationWidgetsAccordingToBasestationModel();

/**
This function toggles which page is displayed in the GUI.  If the current index is 0, it makes it 1 and vice versa.
*/
void toggleGUIPage();

/**
This function updates the visible basestation list and emits the visibleBasestationListChanged signal if the list changes.
@param inputVisibleRegion: A structure defining where on the earth is currently visible in the screen
*/
void regionChanged(const GeoDataLatLonAltBox &inputVisibleRegion);

/**
This function creates a addressBasedLocationReceiver object and uses it to update the map so that it points to the address.
*/
void lookupAddressAndUpdateMap();

/**
This function just emits the signalThatDataModelPotentiallyChanged signal.
*/
void emitDataModelPotentiallyChangedSignal();

/**
This function takes the coordinates of a mouse click on the map and emits basestationSelectedOnMap signals for any basestations that would be selected by that click.
@param inputMouseClickX: The X coordinate of the mouse click on the map
@param inputMouseClickY: The Y coordinate of the mouse click on the map
*/
void processMouseClickOnMap(int inputMouseClickX, int inputMouseClickY);

/**
This slot expands the widget associated with the basestation that was selected on the map.
@param inputMapSelectedBasestationID: The ID the basestation whose widget to expand
*/
void expandMapSelectedBasestation(std::pair<int64_t, int64_t> inputMapSelectedBasestationID);

/**
This function creates a dataReceiver for the given basestation and makes the associated widget.
@return: The ZMQ connection string for the data receiver
*/
std::string addBasestationAsDataSource(std::pair<int64_t, int64_t> inputWidgetSelectedBasestationID);

/**
This function intercepts events that happen to the frame (including mouse hover) and emits the appropriate signals.
@param inputObject: The object that prompted the event signal
@param inputEvent: The event signal
*/
bool eventFilter(QObject *inputObject, QEvent *inputEvent);

/**
This function updates the display of widgets associated with data receivers, according to the current page and the displayOrderToDataReceiverWidgetAddress map.
*/
void updateDataReceiversDisplay();

/**
This function updates the display of widgets associated with data senders according to the displayOrderToDataSenderWidgetAddress map.  Emits updatedDataSendersDisplay() signal when completed.
*/
void updateDataSendersDisplay();

/**
This function removes the data receiver associated with the given casterID/basestationID pair.
@param inputBasestationID: The casterID/basestationID pair
*/
void shutDownBasestationDataReceiver(std::pair<int64_t, int64_t> inputBasestationID);

/**
This function creates a dialog window and connects it so that it can be used to create a new data receiver (which is displayed in the main GUI).
*/
void createDataReceiverDialogWindowForDisplay();

/**
This function deletes the widget associated with this data receiver, cleans out all maps which use the data receiver connection string as their key and removes all data senders (and their widgets) which are connected to this data receiver.
@param inputDataReceiverConnectionString: The data receiver connection string associated with the data receiver
*/
void removeDataReceiverAndItsWidget(const std::string &inputDataReceiverConnectionString);

/**
This function clears the data receivers/data senders from the application.
*/
void removeAllDataReceiversDataSendersAndAssociatedWidgets();

/**
This function deletes the widget associated with this data sender, cleans out all maps which use the data sender connection string as their key.
@param inputDataSenderIDString: The data sender ID string
*/
void removeDataSenderAndItsWidget(const std::string &inputDataSenderConnectionString);

signals:
void signalThatDataModelPotentiallyChanged();
void basestationSelectedOnMap(std::pair<int64_t, int64_t> inputBasestationID);
void dataReceiversUpdated();
void dataSendersUpdated();
void updatedDataSendersDisplay();

};

}

#include "basestationListRetriever.hpp"
