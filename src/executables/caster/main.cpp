#include "Poco/ByteOrder.h"
#include<chrono>

#include "caster.hpp"
#include "caster_configuration.pb.h"

using namespace pylongps;

int main(int argc, char** argv)
{
//Defined options
// -C configurationFile
// -caster_id casterIDNumber
// -transmitter_streaming_port streamingPortNumber 
// -client_request_port clientRequestPortNumber
// -client_streaming_port clientStreamingPortNumber
// -proxy_streaming_publishing_port proxyStreamingPublishingPortNumber
// -stream_status_notification_port streamStatusNotificationPortNumber
// -key_management_port keyManagementPortNumber
// -caster_public_key_path pathToPublicKeyFileForCaster
// -caster_secret_key_path pathToSecretKeyFileForCaster
// -caster_key_management_public_key_path pathToPublicKeyAllowedToSendKeyManagementRequests
// -help list of possible options

std::map<std::string, std::string> processedArguments = parseStringArguments(argv+1, argc-1); //Skip program name

caster_configuration currentConfiguration;
int64_t buffer;

// -help list of possible options
if(processedArguments.count("help") > 0)
{ //Print options and exit
printf("Possible options: \n");
printf("-C configurationFile\n");
printf("-caster_id casterIDNumber\n");
printf("-transmitter_streaming_port streamingPortNumber\n"); 
printf("-client_request_port clientRequestPortNumber\n");
printf("-client_streaming_port clientStreamingPortNumber\n");
printf("-proxy_streaming_publishing_port proxyStreamingPublishingPortNumber\n");
printf("-stream_status_notification_port streamStatusNotificationPortNumber\n");
printf("-key_management_port keyManagementPortNumber\n");
printf("-caster_public_key_path pathToPublicKeyFileForCaster\n");
printf("-caster_secret_key_path pathToSecretKeyFileForCaster\n");
printf("-caster_key_management_public_key_path pathToPublicKeyAllowedToSendKeyManagementRequests\n");
printf("-help get list of possible options\n");
return 0;
}

// -C configurationFile
if(processedArguments.count("C") > 0)
{ //Load configuration file
if(loadProtobufObjectFromFile(processedArguments["C"], currentConfiguration) == false)
{
fprintf(stderr, "Unable to load configuration from file: %s\n", processedArguments["C"].c_str());
return 1;
}
}

// -caster_id casterIDNumber
if(processedArguments.count("caster_id") > 0)
{ //Read ID number
if(convertStringToInteger(processedArguments["caster_id"], buffer) == false)
{
fprintf(stderr, "Unable to read caster_id: %s\n", processedArguments["caster_id"].c_str());
}
currentConfiguration.set_caster_id(buffer);
}

// -transmitter_streaming_port streamingPortNumber 
if(processedArguments.count("transmitter_streaming_port") > 0)
{ //Read port number
if(convertStringToInteger(processedArguments["transmitter_streaming_port"], buffer) == false)
{
fprintf(stderr, "Unable to read transmitter_streaming_port: %s\n", processedArguments["transmitter_streaming_port"].c_str());
}
currentConfiguration.set_transmitter_registration_and_streaming_port_number(buffer);
}

// -client_request_port clientRequestPortNumber
if(processedArguments.count("client_request_port") > 0)
{//Read port number
if(convertStringToInteger(processedArguments["client_request_port"], buffer) == false)
{
fprintf(stderr, "Unable to read client_request_port: %s\n", processedArguments["client_request_port"].c_str());
}
currentConfiguration.set_client_request_port_number(buffer);
}

// -client_streaming_port clientStreamingPortNumber
if(processedArguments.count("client_streaming_port") > 0)
{//Read port number
if(convertStringToInteger(processedArguments["client_streaming_port"], buffer) == false)
{
fprintf(stderr, "Unable to read client_streaming_port: %s\n", processedArguments["client_streaming_port"].c_str());
}
currentConfiguration.set_client_stream_publishing_port_number(buffer);
}

// -proxy_streaming_publishing_port proxyStreamingPublishingPortNumber
if(processedArguments.count("proxy_streaming_publishing_port") > 0)
{//Read port number
if(convertStringToInteger(processedArguments["proxy_streaming_publishing_port"], buffer) == false)
{
fprintf(stderr, "Unable to read proxy_streaming_publishing_port: %s\n", processedArguments["proxy_streaming_publishing_port"].c_str());
}
currentConfiguration.set_proxy_stream_publishing_port_number(buffer);
}

// -stream_status_notification_port streamStatusNotificationPortNumber
if(processedArguments.count("stream_status_notification_port") > 0)
{//Read port number
if(convertStringToInteger(processedArguments["stream_status_notification_port"], buffer) == false)
{
fprintf(stderr, "Unable to read stream_status_notification_port: %s\n", processedArguments["stream_status_notification_port"].c_str());
}
currentConfiguration.set_stream_status_notification_port_number(buffer);
}

// -key_management_port keyManagementPortNumber
if(processedArguments.count("key_management_port") > 0)
{//Read port number
if(convertStringToInteger(processedArguments["key_management_port"], buffer) == false)
{
fprintf(stderr, "Unable to read key_management_port: %s\n", processedArguments["key_management_port"].c_str());
}
currentConfiguration.set_key_registration_and_removal_port_number(buffer);
}

// -caster_public_key_path pathToPublicKeyFileForCaster
if(processedArguments.count("caster_public_key_path") > 0)
{ //Load key file
std::string stringBuffer = loadPublicKeyFromFile(processedArguments["caster_public_key_path"]);
if(stringBuffer.size() == 0)
{
fprintf(stderr, "Unable to read caster_public_key: %s\n", processedArguments["caster_public_key_path"].c_str());
}
currentConfiguration.set_caster_public_key(stringBuffer);
}

// -caster_secret_key_path pathToSecretKeyFileForCaster
if(processedArguments.count("caster_secret_key_path") > 0)
{//Load key file
std::string stringBuffer = loadSecretKeyFromFile(processedArguments["caster_secret_key_path"]);
if(stringBuffer.size() == 0)
{
fprintf(stderr, "Unable to read caster_secret_key_path: %s\n", processedArguments["caster_secret_key_path"].c_str());
}
currentConfiguration.set_caster_secret_key(stringBuffer);
}

// -caster_key_management_public_key_path pathToPublicKeyAllowedToSendKeyManagementRequests
if(processedArguments.count("caster_key_management_public_key_path") > 0)
{//Load key file
std::string stringBuffer = loadPublicKeyFromFile(processedArguments["caster_key_management_public_key_path"]);
if(stringBuffer.size() == 0)
{
fprintf(stderr, "Unable to read caster_key_management_public_key_path: %s\n", processedArguments["caster_key_management_public_key_path"].c_str());
}
currentConfiguration.set_signing_keys_management_key(stringBuffer);
}

//If keys have not been provided, generate them
if(currentConfiguration.caster_public_key().size() == 0 || currentConfiguration.caster_secret_key().size() == 0)
{
std::string casterPublicKey;
std::string casterSecretKey;
std::tie(casterPublicKey, casterSecretKey) = generateSigningKeys();
currentConfiguration.set_caster_public_key(casterPublicKey);
currentConfiguration.set_caster_secret_key(casterSecretKey);
}

if(currentConfiguration.signing_keys_management_key().size() == 0)
{
std::string publicKey;
std::string secretKey;
std::tie(publicKey, secretKey) = generateSigningKeys();
currentConfiguration.set_signing_keys_management_key(publicKey);
}


//Config file should have the options, all modifications completed

//Create ZMQ context
std::unique_ptr<zmq::context_t> context;

SOM_TRY
context.reset(new zmq::context_t);
SOM_CATCH("Error initializing ZMQ context\n")

//Create caster
caster mycaster(context.get(), currentConfiguration);

//Sleep forever while caster operates
while(true)
{
std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

return 0;
}


