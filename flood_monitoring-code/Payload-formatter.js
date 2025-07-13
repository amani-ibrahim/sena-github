function decodeUplink(input) {
  var data = {};

  // Create a DataView to read the byte array
  var view = new DataView(new ArrayBuffer(4));

  // Function to convert 4 bytes to a float
  function bytesToFloat(byteArray, offset) {
    for (let i = 0; i < 4; i++) {
      view.setUint8(i, byteArray[offset + i]);
    }
    return view.getFloat32(0, true); // Little-endian
  }

  // Decode each float from the payload
  data.Distance = bytesToFloat(input.bytes, 0);       
  data.RainStatus = bytesToFloat(input.bytes, 4);        
  
  //ata.temperature = bytesToFloat(input.bytes, 8);    // Ambient Temperature

  return {
    data: data
  };
}
