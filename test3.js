var ZWaveModule = require('openzwave-shared');
var zwave = new ZWaveModule();

console.log(zwave);

try{ zwave.pressButton();             } catch (e) {	console.log(e); }
try{ zwave.pressButton({foo: 'bar'}); } catch (e) {	console.log(e); }
try{ zwave.pressButton({node_id: 2}); } catch (e) {	console.log(e); }
try{ zwave.pressButton({node_id: 2, class_id: 37, instance:1, index: 1}); } catch (e) {	console.log(e); }
