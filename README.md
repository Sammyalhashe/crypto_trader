# Crypto Trading Bot
- The point of this is to have a bot that trades various crytos using various
  strategies.

## Questions
- Is this going to be a long running daemon or something that is fired an
  intervals?
- What language are we using (C++)
- Build system? (CMake with modern approaches)
- Paper trading? (being able to test but making mock trades)
- Backtesting?
- What strategies?
    - Should probably start with a simple strategy.
    - One simple one:
        - start monitoring
        - buy if dips 5%
        - sell if raises 5%
    - arbitrage?
    - market making
- how often can we work on this?
    - dedicate a couple of hours in the workweek (1-2).
    - 4 hours per day (max).
- how can we track progress?
    - trello?
    - basecamp?
- keeping track of resources?
    - does trello do this? Nope.
    - Just put in the repo.
        - But separate into topics to make it easy to look up.
        - A doc section.
- start with ETH?
- CI
    - run unit tests
    - run integration tests with a constant dataset.
