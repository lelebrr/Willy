var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var cryptoData = {
    bitcoin: { price: 0, change: 0, balance: 0 },
    ethereum: { price: 0, change: 0, balance: 0 },
    litecoin: { price: 0, change: 0, balance: 0 },
    ripple: { price: 0, change: 0, balance: 0 }
};

var portfolioValue = 0;
var priceHistory = [];
var maxHistoryPoints = 50;

function drawCryptoWallet(title) {
    display.fill(0);
    display.drawRoundRect(
        5,
        5,
        tftWidth - 10,
        tftHeight - 10,
        5,
        WILLY_PRICOLOR
    );
    display.setTextSize(2);
    display.setTextAlign('center', 'top');
    display.drawText(
        title.length > 20 ? title.substring(0, 20) : title,
        tftWidth / 2,
        5
    );
    display.setTextAlign('left', 'top');
}

function updateCryptoPrices() {
    try {
        cryptoData.bitcoin.price = 50000 + Math.random() * 10000;
        cryptoData.ethereum.price = 3000 + Math.random() * 500;
        cryptoData.litecoin.price = 150 + Math.random() * 50;
        cryptoData.ripple.price = 1 + Math.random() * 0.5;

        cryptoData.bitcoin.change = (Math.random() - 0.5) * 10;
        cryptoData.ethereum.change = (Math.random() - 0.5) * 5;
        cryptoData.litecoin.change = (Math.random() - 0.5) * 3;
        cryptoData.ripple.change = (Math.random() - 0.5) * 1;

        portfolioValue = 0;
        for (var crypto in cryptoData) {
            portfolioValue += cryptoData[crypto].price * cryptoData[crypto].balance;
        }

        priceHistory.push({
            timestamp: Date.now(),
            bitcoin: cryptoData.bitcoin.price,
            ethereum: cryptoData.ethereum.price,
            litecoin: cryptoData.litecoin.price,
            ripple: cryptoData.ripple.price,
            portfolio: portfolioValue
        });

        if (priceHistory.length > maxHistoryPoints) {
            priceHistory.shift();
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function drawCryptoPrices() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    for (var crypto in cryptoData) {
        var data = cryptoData[crypto];
        var color = data.change >= 0 ? 0x00FF00 : 0xFF0000;

        display.setColor(color);
        display.drawText(crypto.toUpperCase() + ': $' + data.price.toFixed(2), 10, y);
        display.drawText('Change: ' + data.change.toFixed(2) + '%', 10, y + 10);
        display.drawText('Balance: ' + data.balance.toFixed(4), 10, y + 20);

        y += 40;
    }

    display.setColor(WILLY_PRICOLOR);
    display.drawText('Portfolio Value: $' + portfolioValue.toFixed(2), 10, y + 10);
}

function drawPriceChart() {
    if (priceHistory.length < 2) return;

    var chartWidth = tftWidth - 20;
    var chartHeight = 80;
    var chartX = 10;
    var chartY = tftHeight - chartHeight - 10;

    display.drawRect(chartX, chartY, chartWidth, chartHeight);

    var maxPrice = Math.max(...priceHistory.map(h => h.portfolio));
    var minPrice = Math.min(...priceHistory.map(h => h.portfolio));
    var priceRange = maxPrice - minPrice;

    display.setColor(0x00FF00);
    for (var i = 1; i < priceHistory.length; i++) {
        var x1 = chartX + (i - 1) * chartWidth / priceHistory.length;
        var y1 = chartY + chartHeight - (priceHistory[i - 1].portfolio - minPrice) * chartHeight / priceRange;
        var x2 = chartX + i * chartWidth / priceHistory.length;
        var y2 = chartY + chartHeight - (priceHistory[i].portfolio - minPrice) * chartHeight / priceRange;

        display.drawLine(x1, y1, x2, y2);
    }

    display.setColor(WILLY_PRICOLOR);
}

function savePortfolio() {
    try {
        storage.write('crypto_portfolio', JSON.stringify(cryptoData), 'write');
        console.log('Portfolio saved successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadPortfolio() {
    try {
        var data = storage.read('crypto_portfolio');
        if (data) {
            cryptoData = JSON.parse(data);
            console.log('Portfolio loaded successfully');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function main() {
    loadPortfolio();

    var running = true;
    var updateInterval = 5000;
    var lastUpdate = 0;

    while (running) {
        var currentTime = Date.now();

        if (currentTime - lastUpdate >= updateInterval) {
            updateCryptoPrices();
            lastUpdate = currentTime;
        }

        drawCryptoWallet('Crypto Wallet');
        drawCryptoPrices();
        drawPriceChart();

        if (keyboard.getSelPress()) {
            var choices = ['Add Bitcoin', 'Add Ethereum', 'Add Litecoin', 'Add Ripple', 'Save Portfolio', 'Load Portfolio', 'Quit'];
            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Save Portfolio') {
                savePortfolio();
            } else if (choice === 'Load Portfolio') {
                loadPortfolio();
            } else {
                var crypto = choice.toLowerCase().replace('add ', '');
                var amount = parseFloat(prompt('Enter amount to add:'));
                if (!isNaN(amount)) {
                    cryptoData[crypto].balance += amount;
                }
            }
        }

        display.draw();
        delay(100);
    }
}

main();
