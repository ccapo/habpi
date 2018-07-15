'use strict';

exports['default'] = {
  xbee: function(api) {
    let config = {
      payload: {
        device: '/dev/ttyAMA0',
        baudRate: 115200,
        destination: '0013A200415B3C95',
        boot: [
          { cmd: 'ATRE', arg: '' },
          { cmd: 'ATNI', arg: 'Green' },
          { cmd: 'ATHP', arg: '0' },
          { cmd: 'ATID', arg: '7FFF' },
          { cmd: 'ATCE', arg: '2' },
          { cmd: 'ATBD', arg: '7' },
          { cmd: 'ATAP', arg: '1' },
          { cmd: 'ATAO', arg: '0' },
          { cmd: 'ATDH', arg: '13A200' },
          { cmd: 'ATDL', arg: '415B3C95' },
          { cmd: 'ATWR', arg: '' }
        ]
      },
      missioncontrol: {
        device: '/dev/cu.usbserial-DN02N149',
        baudRate: 115200,
        destination: '0013A200415B3CA7',
        boot: [
          { cmd: 'ATRE', arg: '' },
          { cmd: 'ATNI', arg: 'Blue' },
          { cmd: 'ATHP', arg: '0' },
          { cmd: 'ATID', arg: '7FFF' },
          { cmd: 'ATCE', arg: '1' },
          { cmd: 'ATBD', arg: '7' },
          { cmd: 'ATAP', arg: '0' },
          { cmd: 'ATAO', arg: '0' },
          { cmd: 'ATDH', arg: '13A200' },
          { cmd: 'ATDL', arg: '415B3CA7' },
          { cmd: 'ATWR', arg: '' }
        ]
      }
    };

    return config[process.env.MODE.toLowerCase()];
  }
};

exports.test = {
  xbee: function(api) {
    return {
      path: '/dev/null',
      boot: []
    };
  }
};

