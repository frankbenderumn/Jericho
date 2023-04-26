/** TODO: 
 * 
 * This class is responsible for managing all thing message related,
 * it should be a component of Bifrost.
 * Target responsibilites TBI are expired messages and persistence/logging
 * Will likely need an observer pattern design to trickle up and down the hierachy
 * since outgoing requests are currently running in separate threads
 * 
 * Hierachy:
 * MessageBroker
 * MessageBuffer
 * Message
 * 
 * MessageQueue..?
 * MessageObserver..?
 * 
*/