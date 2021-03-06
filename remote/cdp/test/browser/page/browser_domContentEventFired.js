/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/ */

"use strict";

const DOC = toDataURL("<div>foo</div>");
const DOC_IFRAME_MULTI = toDataURL(`
  <iframe src='data:text/html,foo'></iframe>
  <iframe src='data:text/html,bar'></iframe>
`);
const DOC_IFRAME_NESTED = toDataURL(`
  <iframe src="${DOC_IFRAME_MULTI}"></iframe>
`);

add_task(async function noEventWhenPageDomainDisabled({ client }) {
  await runContentEventFiredTest(client, 0, async () => {
    info("Navigate to a page with nested iframes");
    await loadURL(DOC_IFRAME_NESTED);
  });
});

add_task(async function noEventAfterPageDomainDisabled({ client }) {
  const { Page } = client;

  await Page.enable();
  await Page.disable();

  await runContentEventFiredTest(client, 0, async () => {
    info("Navigate to a page with nested iframes");
    await loadURL(DOC_IFRAME_NESTED);
  });
});

add_task(async function eventWhenNavigatingWithNoFrames({ client }) {
  const { Page } = client;

  await Page.enable();

  await runContentEventFiredTest(client, 1, async () => {
    info("Navigate to a page with no iframes");
    await loadURL(DOC);
  });
});

add_task(async function eventWhenNavigatingWithFrames({ client }) {
  const { Page } = client;

  await Page.enable();

  await runContentEventFiredTest(client, 1, async () => {
    info("Navigate to a page with iframes");
    await loadURL(DOC_IFRAME_MULTI);
  });
});

add_task(async function eventWhenNavigatingWithNestedFrames({ client }) {
  const { Page } = client;

  await Page.enable();

  await runContentEventFiredTest(client, 1, async () => {
    info("Navigate to a page with nested iframes");
    await loadURL(DOC_IFRAME_NESTED);
  });
});

async function runContentEventFiredTest(client, expectedEventCount, callback) {
  const { Page } = client;

  if (![0, 1].includes(expectedEventCount)) {
    throw new Error(`Invalid value for expectedEventCount`);
  }

  const DOM_CONTENT_EVENT_FIRED = "Page.domContentEventFired";

  const history = new RecordEvents(expectedEventCount);
  history.addRecorder({
    event: Page.domContentEventFired,
    eventName: DOM_CONTENT_EVENT_FIRED,
    messageFn: payload => {
      return `Received ${DOM_CONTENT_EVENT_FIRED} at time ${payload.timestamp}`;
    },
  });

  const timeBefore = Date.now() / 1000;
  await callback();
  const domContentEventFiredEvents = await history.record();
  const timeAfter = Date.now() / 1000;

  is(
    domContentEventFiredEvents.length,
    expectedEventCount,
    "Got expected amount of domContentEventFired events"
  );
  if (expectedEventCount == 0) {
    return;
  }

  const timestamp = domContentEventFiredEvents[0].payload.timestamp;
  ok(
    timestamp >= timeBefore && timestamp <= timeAfter,
    `Timestamp ${timestamp} in expected range [${timeBefore} - ${timeAfter}]`
  );
}
