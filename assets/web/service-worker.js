const cacheName = 'offline';

self.addEventListener('install', (event) => {
  // Cache whole website including the game.
  event.waitUntil(
      caches.open(cacheName).then((cache) => {
        return cache.addAll([
          './',
          './index.html',
          './github-ribbon.svg',
          './buttons/gamepad-l1-button.svg',
          './buttons/virtual-coin-button.svg',
          './buttons/gamepad-y-button.svg',
          './buttons/virtual-thrust-button.svg',
          './buttons/gamepad-dpad-right-button.svg',
          './buttons/gamepad-dpad-up-button.svg',
          './buttons/gamepad-r1-button.svg',
          './buttons/gamepad-x-button.svg',
          './buttons/gamepad-ball-button.svg',
          './buttons/virtual-fire-button.svg',
          './buttons/virtual-1p-button.svg',
          './buttons/virtual-rotate-left-button.svg',
          './buttons/gamepad-dpad-left-button.svg',
          './buttons/virtual-rotate-right-button.svg',
          './buttons/virtual-2p-button.svg',
          './buttons/virtual-hyperspace-button.svg',
          './game.data',
          './game.html',
          './game.js',
          './game.wasm',
        ]);
      }),
  );
});

self.addEventListener('fetch', (event) => {
  // Try responding using the cache and fetch the resource from the network for
  // updating the cache or for responding in case the resource isn't found in
  // the cache.
  // TODO: Return a custom response when the resource isn't avaiable in the
  // cache or the network.
  event.respondWith(
      caches.open(cacheName).then((cache) => {
        return cache.match(event.request).then((response) => {
          const fetchPromise = fetch(event.request).then((netResponse) => {
            cache.put(event.request, netResponse.clone());
            return netResponse;
          });
          return response || fetchPromise;
        });
      }),
  );
});
