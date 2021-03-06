/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const { GeckoViewActorChild } = ChromeUtils.import(
  "resource://gre/modules/GeckoViewActorChild.jsm"
);

const { Services } = ChromeUtils.import("resource://gre/modules/Services.jsm");

const EXPORTED_SYMBOLS = ["TestSupportChild"];

class TestSupportChild extends GeckoViewActorChild {
  receiveMessage(aMsg) {
    debug`receiveMessage: ${aMsg.name}`;

    switch (aMsg.name) {
      case "FlushApzRepaints":
        return new Promise(resolve => {
          const repaintDone = () => {
            debug`APZ flush done`;
            Services.obs.removeObserver(repaintDone, "apz-repaints-flushed");
            resolve();
          };
          Services.obs.addObserver(repaintDone, "apz-repaints-flushed");
          if (this.contentWindow.windowUtils.flushApzRepaints()) {
            debug`Flushed APZ repaints, waiting for callback...`;
          } else {
            debug`Flushing APZ repaints was a no-op, triggering callback directly...`;
            repaintDone();
          }
        });
    }
    return null;
  }
}
const { debug } = TestSupportChild.initLogging("GeckoViewTestSupport");
